
#ifndef SDCARD_H
#define SDCARD_H

void initSD(void);
void readSD(unsigned char* target, char location[12]);
void sendSD(unsigned char *data, unsigned long len);

#endif
