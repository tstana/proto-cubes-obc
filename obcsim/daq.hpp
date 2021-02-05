#ifndef _DAQ_HPP_
#define _DAQ_HPP_


/**
 * daq_init
 * 
 * @brief Initializes the SD-card functions for Proto-CUBES DAQ operation.
 */
void daq_init(void);


/**
 * daq_read_file
 * 
 * @brief Read data from file on SD-card to target buffer
 * 
 * @param location Name of file to read from
 * @param buf Buffer to write data into
 */
void daq_read_file(char location[12], unsigned char *buf);


/**
 * daq_write_new_file
 * 
 * @brief Sends data from buffer to a newly generated file on the SD card.
 *        A new file is generated every time data is sent to it, in the format
 * 	      daqXXXX.dat, where the X is replaced by numbers.
 * 
 * @param data Buffer containing the data to write to the SD card
 * @param len Number of bytes to write to the SD card
 */
void daq_write_new_file(unsigned char *data, unsigned long len);


/**
 * daq_read_last_file
 * 
 * @brief Reads data from the last file on the SD card named in the format
 *        daqXXXX.dat, into the buffer passed as parameter. Also clears the
 *        internal "new_file_available" boolean variable.
 * 
 * @param buf          Data buffer to read data into
 * @param recv_len     Number of bytes read from the file
 */
void daq_read_last_file(char *buf, int *recv_len);


/**
 * daq_new_file_available
 * 
 * @brief Returns the state of the "new_file_available" variable, which
 *        is set in daq_write_new_file() and reset in daq_read_last_file().
 * 
 * @param None
 * 
 * @return The state of the "new_file_available" variable.
 */
bool daq_new_file_available();


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


/**
 * @brief Check whether the Arduino's DAQ_DUR timer has timed out
 * 
 * @param None
 * 
 * @return Boolean value, true when DAQ_DUR timer timed out
 */
boolean daq_data_request_timeout(void);


/**
 * @brief Check whether it is time to sync CUBES and set a new
 *        sync time
 * 
 * @param None
 * 
 * @return Boolean value, true if time synchronization with
 *         CUBES should be done.
 */
boolean daq_sync_timeout(void);


/**
 * @brief Set DAQ_DUR on Arduino, DAQ_DUR sent to CUBES + 1 second
 * 
 * @param cubes_dur DAQ_DUR sent to CUBES, `uint8_t`, in seconds
 * 
 * @return None
 */
void daq_set_dur(uint8_t cubes_dur);


/**
 * @brief Start the Arduino's DAQ_DUR and sync timers
 */
void daq_start_timers(void);


/**
 * @brief Stop the Arduino's DAQ_DUR and sync timers
 */
void daq_stop_timers(void);


/**
 * @brief Check whether DAQ is on-going
 */
bool daq_running(void);


#endif
