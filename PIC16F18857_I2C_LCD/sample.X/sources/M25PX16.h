#ifndef __M25PX16_H_
#define __M25PX16_H_

typedef struct {
    unsigned char manufacturer;
    unsigned char memory_type;
    unsigned char memory_capacity;
    unsigned char cfd_length;
    unsigned char cfd_content[17];
} m25px16_identification_t;

void M25PX16_get_id(m25px16_identification_t *p);
void M25PX16_page_program(unsigned long addr, unsigned char *buf, size_t size);
void M25PX16_read_data_bytes(unsigned long addr, unsigned char *buf, size_t size);
void M25PX16_subsector_erase(unsigned long addr);
void M25PX16_sector_erase(unsigned long addr);

#endif
