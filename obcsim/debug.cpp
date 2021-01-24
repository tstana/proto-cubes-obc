#include <Arduino.h>
#include <string.h>

#include "debug.hpp"
#include "rtc.hpp"

/**
 * @brief Initialize serial port for debug prints
 */
void debug_init(void)
{
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println("==========;==========;==========;==========");
  Serial.println("UTC Time  ;File      ;Line      ;Message   ");
  Serial.println("==========;==========;==========;==========");
}

/**
 * @brief Print custom debug message string to UART
 * 
 * UART0 on the Arduino is used for the print. This function
 * calls `print()` and `println()` functions from the Serial
 * class.
 * 
 * The debug message has the following format:
 * 
 *   <1>;<2>;<3>;<4>\r\n, where:
 *      <1> = Current UTC
 *      <2> = File where this print is issued
 *      <3> = Line in file where this print is issued
 *      <4> = Custom string to print
 * 
 * @param filename Name of the file where this function is called from
 *                 (use `__FILE__` here)
 * @param line     Line in file where this function is called from
 *                 (use `__LINE__` here)
 * @param msg      Message to print to UART port; call `sprintf()`
 *                 before this function to prepare a string that has
 *                 various parameters, like a decimal or a hex number.
 */
void debug_print_string(char *filename, int line, char* msg)
{
    Serial.print(rtc_get_time());
    Serial.print(";");
    /* `strrchr()` strips away all from path but file name */
    Serial.print(1+strrchr(filename, '\\'));
    Serial.print(";");
    Serial.print(line);
    Serial.print(";");
    Serial.print(msg);
    Serial.println();
}
