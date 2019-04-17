
#ifndef SDCARD_H
#define SDCARD_H

void SD_init(void);
void SD_read(unsigned char* target, char location[12]);
void SD_send(unsigned char *data, unsigned long len);

#endif
