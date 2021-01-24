#ifndef __DEBUG_H__
#define __DEBUG_H__

void debug_init(void);
void debug_print_string(char *filename, int line,char *msg);

#define DEBUG_PRINT(m) (debug_print_string(__FILE__, __LINE__, m))

#endif __DEBUG_H__
