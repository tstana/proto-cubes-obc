
#ifndef SDCARD_H
#define SDCARD_H

/*
 * SD_init
 * Initializes the SD-card functions
 */
void SD_init(void);

/*
 * SD_read
 * Reads data from file on SD-card to target buffer
 * Input: Target buffer and file name of reading location
 */
void SD_read(unsigned char* target, char location[12]);

/*
 * SD_send
 * Sends data from buffer to a newly generated file
 * A new file is generated every time data is sent to it, 
 * on the format logxxxxx.txt, where the x is replaced by numbers
 * Input: Data buffer location and length of buffer
 */
void SD_send(unsigned char *data, unsigned long len);

#endif
