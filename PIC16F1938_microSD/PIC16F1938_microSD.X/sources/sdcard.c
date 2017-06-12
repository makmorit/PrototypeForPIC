#include "common.h"
#include "device.h"
#include "sdcard.h"

#define SECTOR_BYTE_NUM 512

#define CMD0   0x00 // カードリセット
#define CMD1   0x01 // MMC初期化
#define CMD8   0x08 // 動作電圧確認／SDCバージョンチェック
#define CMD12  0x0C // データ読込停止
#define CMD13  0x0D // 書込状態照会
#define CMD16  0x10 // ブロックサイズ初期値設定
#define CMD17  0x11 // シングルブロック読込要求
#define CMD24  0x18 // シングルブロック書込要求
#define ACMD41 0x29 // SDC初期化
#define CMD55  0x37 // ACMD41/ACMD23とセットで使用
#define CMD58  0x3A // OCR読出

char mmc_buffer[SECTOR_BYTE_NUM];
int  sdcard_type;
int  fatfs_type;

unsigned long dir_entry_start_pos;
unsigned int  dir_entry_sect_num;
unsigned long data_area_start_pos;
unsigned long fat_area_start_pos;
unsigned int  sector_num_by_cluster;
unsigned long sector_num_per_fat_area;

struct FATFS_PARAMETERS {
    unsigned char jump[3];            // ブート用のジャンプコード
    unsigned char oemId[8];           // フォーマット時のボリューム名
    unsigned int  BytesPerSector;     // １セクタあたりのバイト数、通常は512バイト
    unsigned char SectorsPerCluster;  // １クラスタあたりのセクタ数
    unsigned int  ReservedSectorCount;// ブートセクタ以降の予約領域のセクタ数
    unsigned char FatCount;           // FATの組数(バックアップFAT数)、通常は２組
    unsigned int  RootDirEntryCount;  // ディレクトリの作成可能個数、通常は512個
    unsigned int  TotalSectors16;     // 全領域のセクター総数(FAT12/FAT16用)
    unsigned char MediaType;          // FAT領域の先頭の値、通常は0xF8
    unsigned int  SectorsPerFat16;    // １組のFAT領域が占めるセクタ数(FAT12/FAT16用)
    unsigned int  SectorsPerTrack;    // １トラックあたりのセクタ数
    unsigned int  HeadCount;          // ヘッド数
    unsigned long HidddenSectors;     // 隠蔽されたセクタ数
    unsigned long TotalSectors32;     // 全領域のセクター総数(FAT32用)
    unsigned long SectorsPerFat32;    // １組のFAT領域が占めるセクタ数(FAT32用)
    unsigned int  FlagsFat32;         // FATの有効無効等の情報フラグ
    unsigned int  VersionFat32;
    unsigned long RootDirClusterFat32;// ディレクトリのスタートクラスタ(FAT32用)
    unsigned char Dumy[6];
    unsigned char FileSystemType[8];  // FATの種類("FAT12/16")(FAT32は20バイト下に有る)
    unsigned char BootCode[448];      // ブートコード領域
    unsigned char BootSectorSig0;     // 0x55
    unsigned char BootSectorSig1;     // 0xaa
};

struct DIRECTORY_ENTRY_INFO {
    unsigned char FileName[11];       // ファイル名(8)+拡張子(3)
    unsigned char Attributes;         // ファイルの属性
    unsigned char ReservedNT;         // Windows NT 用 予約領域
    unsigned char CreationTimeTenths; // ファイル作成時間の1/10秒単位をあらわす
    unsigned int  CreationTime;       // ファイルの作成時間(hhhhhmmmmmmsssss)
    unsigned int  CreationDate;       // ファイルの作成日(yyyyyyymmmmddddd)
    unsigned int  LastAccessDate;     // 最終のアクセス日
    unsigned int  FirstClusterHigh;   // データ格納先のFAT番号上位２バイト
    unsigned int  LastWriteTime;      // 最終のファイル書込み時間
    unsigned int  LastWriteDate;      // 最終のファイル書込み日
    unsigned int  FirstClusterLow;    // データ格納先のFAT番号下位２バイト
    unsigned long FileSize;           // ファイルのサイズ
};

static int ready_check()
{
    for (int c = 0; c < 500; c++) {
        if (spi_transmit(0xff) == 0xff) {
            return 0;
        }
        __delay_ms(1);
    }
    return 1;
}

static int send_command_to_mmc(unsigned char cmd, unsigned long arg)
{
    int x , ans;

    spi_ss_select(0);

    ans = ready_check();
    if (ans != 0) {
        printf("send_command: ready_check timed out \r\n");
        return 0xff;
    }

    // コマンド／パラメーター送信
    spi_transmit(cmd | 0x40);
    for (x=24; x >= 0; x -= 8) {
        spi_transmit(arg >> x);
    }

    // CRC送信
    x = 0XFF;
    if (cmd == CMD0) x = 0X95;
    if (cmd == CMD8) x = 0X87;
    spi_transmit(x);

    x = 0;
    do {
        ans = spi_transmit(0xff);
        x++;
    } while (ans & 0x80 && x < 256);

    if (x >= 256) {
        ans = (cmd | 0x40) << 8;    
        printf("send_command: response timed out \r\n");
    } 

    return ans;
}

// 指定のセクタ位置からデータを１ブロック(シングル・ライト)書込
static int sblock_write(unsigned long sector, char *buff)
{
    int i , ans;

    if (sdcard_type != 0x13) {
         // SDHCでない
         sector <<= 9;
    }
    // シングル・リードコマンドの発行
    ans = send_command_to_mmc(CMD24, sector);

    if (ans == 0) {
        // データトークンの送信
        spi_transmit(0xfe);
        for (i=0; i<SECTOR_BYTE_NUM; i++) {
            // データ部の送信
            spi_transmit(*buff);
            buff++;
        }
        // CRCの部分を送信
        spi_transmit(0xff);
        spi_transmit(0xff);
        // レスポンスの受信
        ans = spi_transmit(0xff);
        ans = ans & 0x1F;

        if (ans == 0x05) {
            ans = ready_check();
            if (ans != 0) {
                ans = 0x88;
            } else {
                // 書込みが正常に行われたか問い合わせを行う
                ans = send_command_to_mmc(CMD13, sector);
                i  = spi_transmit(0xff);
                if (ans == 0) {
                    if (i != 0) {
                        // 書込み失敗
                        ans = 0x89;
                    }
                }
            }
        } else {
            // 書き込みエラー
            ans = 0x88;
        }
    } else {
        // CMD24エラー
        ans = CMD24;
    }
    spi_ss_select(1);

    return ans;
}

// 指定のセクタ位置からデータを１ブロック(シングル・リード)読込
static int sblock_read(unsigned long sector, char *buff)
{
    int i , ans;

    // SDHCでない
    if (sdcard_type != 0x13) sector <<= 9;

    // シングル・リードコマンドの発行
    ans = send_command_to_mmc(CMD17, sector);
    if (ans == 0) {
        i = 0;
        // 返信データの１バイト目を待つ
        while(1) {
            ans = spi_transmit(0xff);
            i++;
            if (ans != 0xff || i>=1000) {
                // データトークンタイムアウト
                if (i>=1000) ans = 0x86;
                break;
            }
            __delay_ms(1);
        }
        // ２バイト目以降の受信
        if (ans == 0xfe) {
            for (i=0; i<SECTOR_BYTE_NUM; i++) {
                *buff = spi_transmit(0xff);
                 buff++;
            }
            // CRCの部分を受信
            spi_transmit(0xff);
            spi_transmit(0xff);
            ans = 0;
        } else {
            // 読み込みエラー
            ans = 0x86;
        }
    } else {
        // CMD17エラー
        ans = CMD17;
    }
    spi_ss_select(1);

    return ans;
}

static int read_fatfs_parameter()
{
    union {
        unsigned char c[4];
        unsigned long l;
    } dt;

    struct FATFS_PARAMETERS *fat;
    int i , ans;

    // カードの先頭から５１２バイト読み出す
    ans = sblock_read(0, mmc_buffer);
    if (ans == 0) {
        // BPB(ブートセクタ)までのオフセット値を取出す
        for (i=0; i<4; i++) {
            dt.c[i] = mmc_buffer[i+0x1c6];
        }
        // ＦＡＴファイルシステムのパラメータを読込む
        ans = sblock_read(dt.l,mmc_buffer);
        if (ans == 0) {
            fat = (struct FATFS_PARAMETERS *)mmc_buffer;
            // １クラスタあたりのセクタ数
            sector_num_by_cluster = fat->SectorsPerCluster;
            // １組のFAT領域が占めるセクタ数
            if (fat->SectorsPerFat16 != 0) {
                fatfs_type = 2;
                sector_num_per_fat_area = fat->SectorsPerFat16;
            } else {
                fatfs_type = 4;
                sector_num_per_fat_area = fat->SectorsPerFat32;
            }
            // ＦＡＴ領域の開始セクタ位置
            fat_area_start_pos = fat->ReservedSectorCount + dt.l;
            // ルートディレクトリエントリの開始セクタ位置
            dir_entry_start_pos = fat_area_start_pos + (sector_num_per_fat_area * fat->FatCount);
            // データ領域の開始セクタ位置
            dir_entry_sect_num = fat->RootDirEntryCount / (SECTOR_BYTE_NUM / sizeof(struct DIRECTORY_ENTRY_INFO));
            data_area_start_pos  = dir_entry_start_pos + dir_entry_sect_num;

            // FAT32
            if (fatfs_type == 4) dir_entry_sect_num = sector_num_by_cluster;
        }
    }
    return ans;
}

int sdcard_initialize()
{
    unsigned long arg;
    unsigned char r7[4];
    unsigned int i, ans;

    printf("sdcard_initialize start \r\n");

    sdcard_type = 0;

    // SS=HIGHの状態で、74クロック以上送信し、
    // カードをコマンド待ち状態にする
    spi_ss_select(1);
    for (i = 0; i < 10; i++) {
        spi_transmit(0xFF);
    }

    // カードにリセットコマンド(CMD0)を送信
    ans = send_command_to_mmc(CMD0, 0);
    if (ans == 1) {
        // ＳＤＣ用の初期化処理を行う
        arg = 0;
        // 動作電圧の確認とカードのバージョンチェック
        ans = send_command_to_mmc(CMD8, 0x1AA);
        if (ans == 1) {
            for (i=0; i<4; i++) r7[i] = spi_transmit(0xff);
            if (r7[3] == 0xAA) {
                // SDCver.2のカード
                sdcard_type = 0x12;
                arg = 0X40000000;
            } else {
                // CMD8のエラー
                ans = 0x8200;
            }
        } else {
            // SDCver.1のカード
            if (ans & 0x4) sdcard_type = 0x11;
        }
        if (sdcard_type != 0) {
            // ＳＤＣ用の初期化コマンド(ACMD41)を発行する
            i = 0;
            while(1) {
                ans = send_command_to_mmc(CMD55, 0);
                ans = send_command_to_mmc(ACMD41, arg);
                i++;
                if (ans == 0 || i>=2000) {
                    // ACMD41タイムアウト
                    if (i>=2000) ans = 0x8300|ans;
                    break;
                }
                __delay_ms(1);
            }

            if (ans == 0 && sdcard_type == 0x12) {
                // Ver.2ならOCRの読出しコマンド(CMD58)を発行する
                ans = send_command_to_mmc(CMD58, 0);
                if (ans == 0) {
                    for (i=0; i<4; i++) r7[i] = spi_transmit(0xff);
                    // SDHCのカード
                    if (r7[0] & 0x40) sdcard_type = 0x13;
                } else {
                    // CMD58エラー
                    ans = CMD58 << 8 | ans;
                }
            }
        } else {
            // ＭＭＣ用の初期化コマンド(CMD1)を発行する
            i = 0;
            while(1) {
                ans = send_command_to_mmc(CMD1, 0);
                i++;
                if (ans != 1 || i>=2000) {
                    if (i>=2000) {
                        printf("sdcard_initialize: CMD1 timed out: ans=%d \r\n", ans);
                    }
                    break;
                }
                __delay_ms(1);
            }

            if (ans == 0) {
                // MMC
                sdcard_type = 0x01;
                printf("sdcard_initialize: sdcard_type=MMC \r\n");
            } else {
                printf("sdcard_initialize: CMD1 error: ans=%d \r\n", ans);
            }
        }
        if (ans == 0) {
            // ブロックサイズの初期値を５１２バイトに設定
            ans = send_command_to_mmc(CMD16, SECTOR_BYTE_NUM);
        }
    }
    spi_ss_select(1);
     
    if (ans == 0) {
        ans = read_fatfs_parameter();
    }

    printf("sdcard_initialize end: ans=%d \r\n", ans);
    return ans;
}

// 指定ファイルのエントリ情報を得る、なければエントリの空の場所を得る処理
int search_file(char *filename, FILE_ACCESS_INFO_T *fp)
{
    struct DIRECTORY_ENTRY_INFO *inf;
    int i , j , c , x , ans;

    ans = c = 0;
    fp->dir_entry_index = 0;
    // ディレクトリエントリの全セクタ分だけ繰り返す
    for (i=0; i<dir_entry_sect_num; i++) {
        // ディレクトリエントリのデータを読込む
        ans = sblock_read(dir_entry_start_pos+i,mmc_buffer);
        if (ans == 0) {
            // １セクタ内のエントリの個数分だけ繰り返す
            x = SECTOR_BYTE_NUM / sizeof(struct DIRECTORY_ENTRY_INFO);
            for (j=0; j<x; j++ ) {
                c++;
                // ファイルのエントリを調べる
                inf = (struct DIRECTORY_ENTRY_INFO *)&mmc_buffer[j*sizeof(struct DIRECTORY_ENTRY_INFO)];
                if (inf->FileName[0] == 0x2e) {
                    // サブディレクトリ
                    continue;
                }
                if (inf->FileName[0] == 0xe5) {
                    if (fp->dir_entry_index == 0) fp->dir_entry_index = c;
                    // 削除されたエントリ
                    continue;
                }
                if (inf->FileName[0] == 0x00) {
                    if (fp->dir_entry_index == 0) fp->dir_entry_index = c;
                    i = dir_entry_sect_num;
                    // 検索されなかった
                    ans = 2;
                    // 空のエントリ
                    break;
                }
                // ファイルの属性を調べる
                // 0x00か0x20でないなら普通のファイルでないとする
                if (!((inf->Attributes==0x20)||(inf->Attributes==0x00))) continue;
                // ファイル名を比較する
                if (memcmp(inf->FileName,filename,11) == 0) {
                    fp->dir_entry_index = c;
                    // ファイルのアクセス情報を設定する
                    // ファイルの読込む位置は０
                    fp->file_seek_pos = 0;
                    // ファイルのサイズ
                    fp->file_size = inf->FileSize;
                    // データ格納先のFAT番号を記録
                    fp->first_fat_no = inf->FirstClusterHigh;
                    fp->first_fat_no = (fp->first_fat_no<<16) | inf->FirstClusterLow;
                    i = dir_entry_sect_num;
                    ans = 1;
                    // 正常終了
                    break;
                }
            }
        } else ans = -1;
    }

    return ans;
}

unsigned long search_fat()
{
    unsigned long ans;
    unsigned int i;
    int j , x , k;

    ans = 0;
    for (i=0; i<sector_num_per_fat_area; i++) {
        if (sblock_read(fat_area_start_pos+i,mmc_buffer) == 0) {
            for (j=0; j<SECTOR_BYTE_NUM; j=j+fatfs_type) {
                // 空きのＦＡＴ
                x = 0;
                for (k=0; k<fatfs_type; k++) x = x | mmc_buffer[j+k];
                if (x == 0) {
                    mmc_buffer[j]   = 0xff;
                    mmc_buffer[j+1] = 0xff;
                    if (fatfs_type == 4) {
                        // FAT32
                        mmc_buffer[j+2] = 0xff;
                        mmc_buffer[j+3] = 0x0f;
                    }
                    // 予約更新
                    if (sblock_write(fat_area_start_pos+i,mmc_buffer) == 0) {
                        // ２組目のFATも書込む
                        sblock_write((fat_area_start_pos+i)+sector_num_per_fat_area,mmc_buffer);
                        ans = (i * SECTOR_BYTE_NUM + j) / fatfs_type;
                    }
                    i = sector_num_per_fat_area;
                    // 検索終了
                    break;
                }
            }
        } else break;
    }
    return ans;
}

int direntry_make(unsigned long no,char *filename, FILE_ACCESS_INFO_T *fp)
{
    struct DIRECTORY_ENTRY_INFO *inf;
    unsigned long p;
    unsigned int x , y;
    int ans;

    ans = -1;
    // ディレクトリエントリを読込む
    x = fp->dir_entry_index - 1;
    y = SECTOR_BYTE_NUM / sizeof(struct DIRECTORY_ENTRY_INFO);
    p = dir_entry_start_pos + (x / y);
    if (sblock_read(p,mmc_buffer) == 0) {
        inf = (struct DIRECTORY_ENTRY_INFO *)&mmc_buffer[(x % y) * sizeof(struct DIRECTORY_ENTRY_INFO)];
        if (no != 0) {
            memset(inf,0x00,sizeof(struct DIRECTORY_ENTRY_INFO));
            // ファイル名を設定する
            memcpy(inf->FileName,filename,11);
            // ファイルの属性を設定する
            inf->Attributes = 0x20;
            // ファイルの新規作成
            inf->FileSize = 0;
            // ファイルの作成時間を設定する
            inf->CreationTimeTenths = 0;
            inf->CreationTime       = 0;
            inf->LastWriteTime      = 0;
            // ファイルの作成日を設定する
            inf->CreationDate  = 0;
            inf->LastWriteDate = 0;
            // アクセス日を設定する
            inf->LastAccessDate = 0;
            // データ格納先のFAT番号を設定する
            inf->FirstClusterHigh = (unsigned int)(no >> 16);
            inf->FirstClusterLow  = (unsigned int)(no & 0x0000ffff);
            // ファイルのアクセス情報を設定する
            fp->file_seek_pos = 0;
            fp->file_size  = 0;
            // データ格納先のFAT番号を記録
            fp->first_fat_no = inf->FirstClusterHigh;
            fp->first_fat_no = (fp->first_fat_no<<16) | inf->FirstClusterLow;
        } else {
            // ファイルのサイズを更新する
            inf->FileSize = inf->FileSize + fp->append_file_size;
            // TODO: ファイル書込み日時を設定する
            // TODO: アクセス日を設定する
        }
        // ディレクトリエントリの更新
        ans = sblock_write(p,mmc_buffer);
    }
    return ans;
}

void filename_check(char *c,const char *filename)
{
    int i;

    memset(c,0x20,11);
    for (i=0; i<8; i++) {
        if (*filename == '.') {
            c = c + (8-i);
            filename++;
            break;
        }
        *c = *filename;
        c++;
        filename++;
    }
    if (i > 7) filename++;
    for (i=0; i<3; i++) {
        if (*filename == 0x00) break;
        *c = *filename;
        c++;
        filename++;
    }
}

int sdcard_open_file(FILE_ACCESS_INFO_T *fp,const char *filename,int oflag)
{
    unsigned long no;
    char c[11];
    int  ans , ret;

    ret = -1;
    fp->file_open_flag = 0;

    // 指定のファイル名を成形する
    filename_check(c,filename);
    // 指定されたファイルを検索する
    ans = search_file(c,(FILE_ACCESS_INFO_T *)fp);
    if (ans > 0) {
        if (oflag == OP_READ_ONLY && ans == 1) ret = 0;
        if (oflag == OP_APPEND && ans == 1) {
            // ファイルのアクセス位置をファイルの最後＋１に設定する
            fp->file_seek_pos = fp->file_size;
            ret = 0;
        }
        if (oflag == OP_READ_WRITE && (ans == 1 || ans == 2)) {
            if (ans == 2) {
                // 新規ファイルの作成を行う
                if (fp->dir_entry_index != 0) {
                    // 空きのFATを探して確保する
                    no = search_fat();
                    if (no != 0) {
                        // ディレクトリエントリの作成を行う
                        if (direntry_make(no,c, (FILE_ACCESS_INFO_T *)fp) == 0) ret = 0;
                    }
                }
            } else ret = 0;
        }
        if (ret == 0) {
            // ファイルにアクセス可能とする
            fp->file_open_flag = oflag;
            fp->append_file_size = 0;
        }
    }

    return ret;
}

void sdcard_close_file(FILE_ACCESS_INFO_T *fp)
{
    // 書き込みオープン時の処理
    if (fp->file_open_flag & (OP_APPEND | OP_READ_WRITE)) {
        if (fp->append_file_size == 0) return;
        // ディレクトリエントリの更新を行う
        direntry_make(0, 0, (FILE_ACCESS_INFO_T *)fp);
    }
    // ファイルのアクセス情報を初期化する
    fp->file_open_flag = 0;
}

// 指定されたFAT番号の次のFAT番号を得る処理
unsigned long next_fat_read(unsigned long fatno, FILE_ACCESS_INFO_T *fp)
{
    union {
        unsigned char c[4];
        unsigned long i;
    } no;
    unsigned long p , x , y , ans;
    int j;

    // ＦＡＴ領域のデータを読込む
    p = fat_area_start_pos + (fatno / (SECTOR_BYTE_NUM/fatfs_type));
    ans = sblock_read(p,mmc_buffer);
    if (ans == 0) {
        x = (fatno % (SECTOR_BYTE_NUM/fatfs_type)) * fatfs_type;
        no.i = 0;
        for (j=0; j<fatfs_type; j++) no.c[j] = mmc_buffer[x+j];
        // 次のチェーン先FAT番号を得る
        ans = no.i;

        // FAT32
        if (fatfs_type == 4) y = 0x0fffffff;
        else              y = 0xffff;

        // 次のチェーン先がない時は新規にチェーン先ＦＡＴを作成する
        if (y == ans) {
            // 新ＦＡＴ番号の空きを探す
            ans = search_fat();
            if (ans != 0) {
                // チェーン元のＦＡＴ情報を更新する
                if (sblock_read(p,mmc_buffer) == 0) {
                    no.i = ans;
                    for (j=0; j<fatfs_type; j++) mmc_buffer[x+j] = no.c[j];
                    if (sblock_write(p,mmc_buffer) == 0) {
                        // ２組目のFATも書込む
                        sblock_write(p + sector_num_per_fat_area,mmc_buffer);
                    } else ans = 0;
                } else ans = 0;
            }
        }
    } else ans = 0;

    return ans;
}

// シーク位置からFAT番号(クラスタ番号)を算出する処理
void fatno_seek_conv(unsigned long *fatno, FILE_ACCESS_INFO_T *fp)
{
    unsigned int  p;
    int  i;

    // データのシーク位置から読出論理セクタ位置を算出し何番目の論理クラスタ位置か？
    p = (fp->file_seek_pos / SECTOR_BYTE_NUM) / sector_num_by_cluster;
    // FAT領域より論理クラスタから実際のFAT番号(クラスタ番号)を算出する
    *fatno = fp->first_fat_no;
    for (i=0; i<p; i++) {
        // 次のチェーン先FAT番号を読込んでおく
        *fatno = next_fat_read(*fatno,(struct FILE_ACCESS_INFO *)fp);
    }
}

// この関数が実際にファイルから指定したバイト数だけ読み込む処理
int sd_rdwr(char *buf, int nbyte, int type, FILE_ACCESS_INFO_T *fp)
{
    unsigned long dtSP;
    unsigned long fatno;
    unsigned int  p , x;
    int  i , c , ans;

    // シーク位置からFAT番号(クラスタ番号)を算出
    fatno_seek_conv(&fatno,(FILE_ACCESS_INFO_T *)fp);
    // FAT領域読込みエラー
    if (fatno == 0) return -1;

    // データの先頭セクタ位置の算出
    dtSP = data_area_start_pos + ((fatno - 2) * sector_num_by_cluster);
    // クラスタ内のセクタ位置
    p = (fp->file_seek_pos / SECTOR_BYTE_NUM) % sector_num_by_cluster;

    // データ領域からファイル内容を読出す
    ans = sblock_read(dtSP+p,mmc_buffer);
    if (ans == 0) {
        x = fp->file_seek_pos % SECTOR_BYTE_NUM;
        c = 0;
        // 指定バイト数ぶん繰り返す
        for (i=0; i<nbyte; i++ ) {
            // 書込み
            if (type == 3) mmc_buffer[x] = *buf;
            // 読込み
            else           *buf = mmc_buffer[x];
            c++;
            x++;
            fp->file_seek_pos++;
            if (fp->file_seek_pos >= fp->file_size) {
                // 最後まで読み込んだ
                if (type < 3) break;
                // データが追加された分だけカウント
                fp->append_file_size++;
            }

            // SECTER_BYTESだけ処理した
            if (c >= SECTOR_BYTE_NUM) break;

            // ＬＦなら終了
            if (type == 2 && *buf == 0x0a) break;

            // 次のセクタにデータがまたがった場合の処理
            if (x >= SECTOR_BYTE_NUM) {
                // 書込み要求ならここまでのデータを書込む
                if (type == 3) {
                    ans = sblock_write(dtSP+p,mmc_buffer);
                    if (ans != 0) {
                        // データ領域書込みエラー
                        ans = -1;
                        break;
                    }
                }
                p++;
                if (p >= sector_num_by_cluster) {
                    // 次のクラスタにデータが有るようである
                    // シーク位置から次のFAT番号(クラスタ番号)を算出
                    fatno_seek_conv(&fatno,(struct FILE_ACCESS_INFO *)fp);
                    if (fatno == 0) {
                        // FAT領域読込みエラー
                        ans = -1;
                        break;
                    }
                    // データの先頭セクタ位置の算出
                    dtSP = data_area_start_pos + ((fatno - 2) * sector_num_by_cluster);
                    // クラスタ内のセクタ位置
                    p = (fp->file_seek_pos / SECTOR_BYTE_NUM) % sector_num_by_cluster;
                }
                // 次のブロックを読込む
                ans = sblock_read(dtSP+p,mmc_buffer);
                if (ans == 0) x = 0;
                else {
                    // データ領域読込みエラー
                    ans = -1;
                    break;
                }
            }
            buf++;
        }
        // 書込み要求ならデータを書込む
        if (x != 0 && ans != -1 && type == 3) {
             ans = sblock_write(dtSP+p,mmc_buffer);
             if (ans != 0) {
                 // データ領域書込みエラー
                 ans = -1;
             }
        }
        if (ans != -1) {
            // 読書込んだバイト数を返す
            ans = c;
        }
    } else {
        // データ領域読込みエラー
        ans = -1;
    }

    return ans;
}

int sdcard_write_line(FILE_ACCESS_INFO_T *fp,const unsigned char *buf,int nbyte)
{
    char *p = (char *)buf;

    if ((fp->file_open_flag & (OP_APPEND | OP_READ_WRITE)) == 0) {
        return -1;
    }
    
    return sd_rdwr(p, nbyte, 3, (FILE_ACCESS_INFO_T *)fp);
}

int sdcard_read_line(FILE_ACCESS_INFO_T *fp,char *buf,int nbyte)
{
    if ((fp->file_open_flag & (OP_READ_ONLY | OP_READ_WRITE)) == 0) {
        return -1;
    }
    
    if (fp->file_seek_pos >= fp->file_size) {
        // EOF
        return  0;
    }
    
    return sd_rdwr(buf, nbyte, 2, (FILE_ACCESS_INFO_T *)fp);
}
