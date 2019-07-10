
#ifndef SDCARD_H
#define SDCARD_H

/*
 * daq_init
 * Initializes the SD-card functions for Proto-CUBES DAQ operation.
 */
void daq_init(void);

/*
 * SD_read
 * Reads data from file on SD-card to target buffer
 * Input: Target buffer and file name of reading location
 */
void SD_read(unsigned char* target, char location[12]);

/*
 * SD_write_new_daq_file
 * Sends data from buffer to a newly generated file
 * A new file is generated every time data is sent to it, 
 * on the format logxxxxx.txt, where the x is replaced by numbers
 * Input: Data buffer location and length of buffer
 */
void daq_write_new_file(unsigned char *data, unsigned long len);

void daq_read_last_file(void);

#endif
