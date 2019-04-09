/*
 * msp_obc_error.h
 * Author: John Wikman
 *
 * Contains definitions for all the internal error codes used for the OBC side
 * of this MSP implementation.
 */

#ifndef MSP_OBC_ERROR_H
#define MSP_OBC_ERROR_H

/* Link Errors */
#define MSP_OBC_ERR_INVALID_OPCODE -1
#define MSP_OBC_ERR_LENGTH_NOT_ZERO -2
#define MSP_OBC_ERR_NOT_IN_A_TRANSACTION -3
#define MSP_OBC_ERR_INVALID_LENGTH -4
#define MSP_OBC_ERR_NULL_POINTER -5
#define MSP_OBC_ERR_INVALID_ACTION -6
#define MSP_OBC_ERR_INVALID_STATE -7

#define MSP_OBC_ERR_INVALID_FRAME -8
#define MSP_OBC_ERR_I2C_ERROR -9

/* Transaction Errors */
#define MSP_OBC_ERR_TRANSACTION_ABORTED -10

/* Transaction Successes */
#define MSP_OBC_TRANSACTION_SUCCESSFUL 1
#define MSP_OBC_RECEIVED_DUPLICATE_TRANSACTION 2

#endif
