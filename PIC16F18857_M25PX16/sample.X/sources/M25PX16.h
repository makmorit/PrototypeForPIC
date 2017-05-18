#ifndef __M25PX16_H_
#define __M25PX16_H_

void M25PX16_page_program(unsigned long addr, unsigned char *buf, size_t size);
void M25PX16_read_data_bytes(unsigned long addr, unsigned char *buf, size_t size);
void M25PX16_sector_erase(unsigned long addr);

#endif // __M25PX16_H_