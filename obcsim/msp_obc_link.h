/*
 * msp_obc_link.h
 * Author: John Wikman
 *
 * Defines functions for sending and receiving data over MSP from an OBC
 * perspective. Also defines a link struct for keeping track of the state
 * towards an experiment. The link struct includes variables that keep track of
 * things such as the address of the experiment, previous transactions-ID's,
 * the current transaction-ID, the opcode of the current transaction, etc.
 */

#ifndef MSP_OBC_LINK_H
#define MSP_OBC_LINK_H

#include "msp_seqflags.h"

typedef enum {
	MSP_LINK_STATE_READY,
	MSP_LINK_STATE_SEND_TX_HEADER,
	MSP_LINK_STATE_SEND_TX_DATA,
	MSP_LINK_STATE_REQ_TX_HEADER,
	MSP_LINK_STATE_REQ_RX_RESPONSE,
	MSP_LINK_STATE_REQ_RX_DATA
} msp_link_state_t;

typedef enum {
	MSP_LINK_ACTION_DO_NOTHING,
	MSP_LINK_ACTION_TX_HEADER,
	MSP_LINK_ACTION_TX_DATA,
	MSP_LINK_ACTION_RX_HEADER,
	MSP_LINK_ACTION_RX_DATA
} msp_link_action_t;

typedef enum {
	MSP_RESPONSE_OK,
	MSP_RESPONSE_BUSY,
	MSP_RESPONSE_TRANSACTION_SUCCESSFUL,
	MSP_RESPONSE_TRANSACTION_ABORTED,
	MSP_RESPONSE_ERROR
} msp_response_type_t;

/* A struct that keeps track of a link to an experiment. */
typedef struct msp_link {
	/*** Link Specific Variables ***/
	/* Experiment Address */
	unsigned char slave_address;

	/* MSP Frame Buffer and MTU */
	unsigned char *buffer;
	unsigned long mtu;

	/* Sequence flags for remembering previous transaction id's */
	msp_seqflags_t flags;

	/*** Transaction specific variables ***/
	msp_link_state_t state;
	msp_link_action_t next_action;
	int corrupt_frame_count;

	unsigned char transaction_id;
	unsigned char frame_id;

	unsigned char opcode;

	/* Total number of bytes to be sent/received and the number of bytes that
	 * are sent/received so far. */
	unsigned long total_length;
	unsigned long processed_length;
} msp_link_t;

/*
 * A struct that contains information about how a Link function call went. If
 * the type if the response is not MSP_OBC_RESPONSE_TRANSACTION_SUCCESSFUL,
 * then the values of the other 3 fields are undefined.
 */
struct msp_response {
	msp_response_type_t status;
	int error_code;
	unsigned char opcode;
	unsigned char transaction_id;
	unsigned long len;
};

/* Creates a link to an experiment. Does not need to be recreated when
 * communicating with the same experiment anew. */
msp_link_t msp_create_link(unsigned long slave_address,
                           msp_seqflags_t flags,
                           unsigned char *buf,
                           unsigned long mtu);

/* Starts a transaction with the given opcode. Does not send an actual frame,
 * it only sets the state of the link to be in a transaction type. This
 * functions is non-blocking. */
struct msp_response msp_start_transaction(msp_link_t *lnk,
                                          unsigned char opcode,
                                          unsigned long len);

/* Aborts the current transaction by sending a null frame to the experiment. */
struct msp_response msp_abort_transaction(msp_link_t *lnk);

/* Sends a data frame. */
struct msp_response msp_send_data_frame(msp_link_t *lnk,
                                        unsigned char *data,
                                        unsigned long datalen);

/* Sends a header frame. */
struct msp_response msp_send_header_frame(msp_link_t *lnk);

/* Receives a data frame. */
struct msp_response msp_recv_data_frame(msp_link_t *lnk,
                                        unsigned char *data,
                                        unsigned long *datalen);

/* Receives a header frame. */
struct msp_response msp_recv_header_frame(msp_link_t *lnk);


/*** Helper functions ***/
/* Returns 1 if the link is in a transaction, otherwise 0. */
int msp_is_active(const msp_link_t *lnk);

/* Returns the next action that the OBC should perform in an MSP transaction */
msp_link_action_t msp_next_action(const msp_link_t *lnk);

/* The amount of data to be sent in the next data frame or be received from the
 * next data/frame */
unsigned long msp_next_data_length(const msp_link_t *lnk);

/* Returns the offset of the next data to be sent/received */
unsigned long msp_next_data_offset(const msp_link_t *lnk);

/* Returns the number of frame error that has occurred in the current
 * transaction */
int msp_error_count(const msp_link_t *lnk);

#endif
