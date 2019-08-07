#ifndef _DAQ_HPP_
#define _DAQ_HPP_

/*
 * daq_init
 * Initializes the SD-card functions for Proto-CUBES DAQ operation.
 */
void daq_init(void);

/*
 * daq_read_file
 * Reads data from file on SD-card to target buffer
 * Inputs: File name of reading location and target buffer
 */
void daq_read_file(char location[12], unsigned char *buf);

/*
 * daq_write_new_file
 * Sends data from buffer to a newly generated file
 * A new file is generated every time data is sent to it, 
 * on the format daqXXXX.dat, where the X is replaced by numbers
 * Input: Data buffer location and length of buffer
 */
void daq_write_new_file(unsigned char *data, unsigned long len);

/**
 * daq_read_last_file
 * 
 * @brief Reads data from the last file on the SD card named in the format
 *        daqXXXX.dat, into the buffer passed as parameter
 * 
 * @param *buf          Data buffer to read data into
 *        *recv_len     Number of bytes read from the file
 */
void daq_read_last_file(char *buf, int *recv_len);

/**
 * daq_delete_all_files
 * 
 * @brief   Removes all files from the SD card, starting with counting down 
 *          the file number and then clearing remaining files.
 * 
 * @param None
 * 
 * @return Integer with number of files deleted.
 */

int daq_delete_all_files(void);

#endif
