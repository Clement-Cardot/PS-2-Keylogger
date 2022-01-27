/*
 * Bluetooth.h
 *
 *  Created on: 12 déc. 2021
 *      Author: Clement CARDOT
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "stm32f1_uart.h"

bool_e isBluetoothPaired(void);
bool_e isBluetoothBufferEmpty(void);
uint8_t getNextCharFromBluetoothBuffer(void);
void addToBluetoothBuffer(uint8_t c);
void sendString(uint8_t * c, uint32_t len);
void sendChar(char c);

#endif /* BLUETOOTH_H_ */
