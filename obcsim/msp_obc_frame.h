/*
 * msp_obc_frame.h
 * Author: John Wikman
 *
 * Defines a struct for handling frames in the OBC side of MSP along with
 * corresponding encoding and decoding functions.
 */

#ifndef MSP_OBC_FRAME_H
#define MSP_OBC_FRAME_H

#include "msp_obc_link.h"

typedef enum {
	MSP_OBC_FRAME_ERROR,
	MSP_OBC_FRAME_HEADER,
	MSP_OBC_FRAME_DATA
} msp_obc_frametype_t;

struct msp_obc_frame {
	msp_obc_frametype_t type;

	unsigned char id;
	unsigned char opcode;

	/* The data field and its length */
	unsigned char *data;
	unsigned long datalen;

	/* Value of the DL field for headers */
	unsigned long dl;
};


struct msp_obc_frame msp_obc_decode_frame(const msp_link_t *lnk, unsigned char *src, unsigned long len);

int msp_obc_encode_frame(const msp_link_t *lnk, unsigned char *dest, unsigned long *len, struct msp_obc_frame frame);

#endif