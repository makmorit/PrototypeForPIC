#ifndef __M25PX16_H_
#define __M25PX16_H_

typedef struct {
    unsigned char manufacturer;
    unsigned char memory_type;
    unsigned char memory_capacity;
    unsigned char cfd_length;
    unsigned char cfd_content[17];
} m25px16_identification_t;

void M25PX16_init();
void M25PX16_get_id(m25px16_identification_t *p);

void spi_intr();

#endif
