/*
 * msp_i2c_master.h
 *
 * Defines functions to communicate over I2C from a master perspective. Some of
 * these functions are required to be implemented by the MSP library. It is
 * written in the prototype comments if a function is required to be
 * implemented.
 */

#ifndef MSP_I2C_MASTER_H
#define MSP_I2C_MASTER_H

/*
 * NOTE:
 * This function is not used by the MSP library. This function is not necessary
 * to implement if there are other ways of setting the I2C timeout.
 *
 * Start the I2C driver. Non-blocking call.
 *
 * Params:
 *  - i2c_clockrate: I2C bus speed in Hz
 *  - i2c_timeout: Timeout rate.
 *
 * Returns 0 on success, a negative value between -1 and -9 on failure.
 */
int msp_i2c_start(unsigned long i2c_clockrate, unsigned long i2c_timeout);

/*
 * NOTE:
 * This function is not used by the MSP library. This function is not necessary
 * to implement if there are other ways of setting the I2C timeout.
 *
 * Sets the timeout value of the I2C driver. Non-blocking call.
 *
 * Params:
 *  - i2c_timeout: Timeout rate.
 */
void msp_i2c_set_timeout(unsigned long i2c_timeout);

/*
 * NOTE:
 * This function is not used by the MSP library. It is not necessary to
 * implement this function if there are other ways of stopping the I2C driver.
 *
 * Stops the I2C driver. Non-blocking call.
 */
void msp_i2c_stop(void);

/*
 * REQUIRED BY THE MSP DRIVER. MUST BE IMPLEMENTED.
 *
 * NOTE:
 * The data pointer here should actually be const. But to ensure that the
 * code works with other frameworks, the const is omitted here.
 *
 * Writes data to a slave on the I2C bus. Blocking call.
 *
 * Params:
 *  - slave_address: I2C address of the slave.
 *  - data: Pointer to the data to be written to the slave.
 *  - size: Number of bytes from data to be written to the slave.
 *
 * Returns 0 on success, a value between -1 and -9 if an error occurred before
 * initiating communication on the I2C bus, and a value between 1 and 9 if an
 * error occurred during communication on the I2C bus.
 */
int msp_i2c_write(unsigned long slave_address, unsigned char *data, unsigned long size);

/*
 * REQUIRED BY THE MSP DRIVER. MUST BE IMPLEMENTED.
 *
 * Reads data from a slave on the I2C bus. Blocking call.
 *
 * Params:
 *  - slave_address: I2C address of the slave.
 *  - data: Pointer to a buffer where the received data will be stored.
 *  - size: Number of bytes of data to be read from the slave.
 *
 * Returns 0 on success, a value between -1 and -9 if an error occurred before
 * initiating communication on the I2C bus, and a value between 1 and 9 if an
 * error occurred during communication on the I2C bus.
 */
int msp_i2c_read(unsigned long slave_address, unsigned char *data, unsigned long size);

#endif
