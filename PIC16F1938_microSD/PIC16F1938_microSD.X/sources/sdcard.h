#ifndef _SDCARD_H_
#define _SDCARD_H_

#define OP_READ_ONLY  0b0001
#define OP_READ_WRITE 0b0010
#define OP_APPEND     0b0100

typedef struct FILE_ACCESS_INFO {
     unsigned int  file_open_flag;
     unsigned int  dir_entry_index;
     unsigned long file_size;
     unsigned long file_seek_pos;
     unsigned long append_file_size;
     unsigned long first_fat_no;
} FILE_ACCESS_INFO_T;

int sdcard_initialize();
int sdcard_open_file(FILE_ACCESS_INFO_T *fp, const unsigned char *filename, int oflag);
void sdcard_close_file(FILE_ACCESS_INFO_T *fp);
int sdcard_write_line(FILE_ACCESS_INFO_T *fp, const unsigned char *buf, int nbyte);
int sdcard_read_line(FILE_ACCESS_INFO_T *fp, unsigned char *buf, int nbyte);

#endif // _SDCARD_H_
