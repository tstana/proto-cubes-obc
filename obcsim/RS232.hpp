/*
 * RS_init
 * Initializes the RS232 communication by opening the Serial port
 */

void RS_init(void);

/* RS_read 
 *  Gets called when signaled that there is data on the Serial line
 *  Reads the first character for type of command and then executes the command
 *  Input is the msp link, to be able to call the invoke_ commands from msp
 */
void RS_read(msp_link_t *lnk);

/* RS_send
 *  Sends the data from the OBC over the Serial line to the RS232.
 *  Takes buffer array and length of buffer as input arguments.
 */
void RS_send(unsigned char *sends, int len);

boolean is_daq_on(void);
