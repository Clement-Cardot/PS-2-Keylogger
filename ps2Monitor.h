/*
 * PS_2.h
 *
 *  Created on: 12 déc. 2021
 *      Author: Clement
 */

#ifndef PS2MONITOR_H_
#define PS2MONITOR_H_
#include <stdint.h>

static volatile uint8_t timeout;

void ps2Interrupt(void);
uint8_t scancodeToChar(uint8_t scancode);

#endif /* PS2MONITOR_H_ */
