/*
 * msp_obc_frame.c
 * Author: John Wikman
 *
 * Implements functionalities for encoding and decoding frames on the OBC
 * side of MSP.
 */

#include "msp_crc.h"
#include "msp_endian.h"
#include "msp_opcodes.h"
#include "msp_obc_error.h"
#include "msp_obc_frame.h"
#include "msp_obc_link.h"

/*
 * Decodes the frame of length len located in src. The context in which the
 * frame is to be decoded is specified by lnk.
 *
 * If decoding the frame failed, it will be specified in the type field of the
 * frame.
 */
struct msp_obc_frame msp_obc_decode_frame(const msp_link_t *lnk, unsigned char *src, unsigned long len)
{
    struct msp_obc_frame frame;
    unsigned long remainder;
    unsigned long fcs;
    unsigned char pseudo_header;

    /* Set the frame as erroneous by default */
    frame.type = MSP_OBC_FRAME_ERROR;

    if (len < 6)
        return frame;

    pseudo_header = (lnk->slave_address << 1) | 0x01;

    remainder = crc32(&pseudo_header, 1, 0);
    remainder = crc32(src, len - 4, remainder);

    fcs = from_bigendian32(src + (len - 4));

    if (fcs != remainder)
        return frame;

    frame.opcode = src[0] & 0x7F;
    frame.id = (src[0] >> 7) & 0x01;

    if (frame.opcode == MSP_OP_DATA_FRAME) {
        frame.type = MSP_OBC_FRAME_DATA;
        frame.data = src + 1;
        frame.datalen = len - 5;
    } else if (len == 9) { /* A header frame must have len == 9 */
        frame.type = MSP_OBC_FRAME_HEADER;
        frame.dl = from_bigendian32(src + 1);
    }

    return frame;
}

/*
 * Encodes the entered frame into the buffer pointed to be dest. The length of
 * the encoded frame is written into the long integer pointed to by len. The
 * context of which the frame is to be encoded in is specified by lnk.
 *
 * It is important that the dest buffer holds at least 5 + the length of the
 * data in the frame or 9 bytes in case of a header frame.
 *
 * Returns 0 if the encoding was ok. If the encoding of the frame failed, an
 * error code from "msp_obc_error.h" is returned.
 */
int msp_obc_encode_frame(const msp_link_t *lnk, unsigned char *dest, unsigned long *len, struct msp_obc_frame frame)
{
    unsigned long i;
    unsigned long fcs;
    unsigned char pseudo_header;

    /* Do not encode erroneous frames */
    if (frame.type == MSP_OBC_FRAME_ERROR)
        return MSP_OBC_ERR_INVALID_FRAME;

    dest[0] = frame.opcode | ((frame.id << 7) & 0x80);

    if (frame.type == MSP_OBC_FRAME_DATA) {
        for (i = 0; i < frame.datalen; i++)
            dest[i + 1] = frame.data[i];
        
        *len = frame.datalen + 5;
    } else {
        to_bigendian32(dest + 1, frame.dl);
        *len = 9;
    }

    pseudo_header = (lnk->slave_address << 1);

    fcs = crc32(&pseudo_header, 1, 0);
    fcs = crc32(dest, *len - 4, fcs);

    to_bigendian32(dest + (*len - 4), fcs);

    return 0;
}
