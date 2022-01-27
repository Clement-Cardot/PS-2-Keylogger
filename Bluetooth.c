/**
  ******************************************************************************
  * @file    Bluetooth.c
  * @author  Clément CARDOT
  * @date    11-dec-2021
  * @brief   Bluetooth communication fonctions.
  ******************************************************************************
*/
#include <stdint.h>
#include "stm32f1_uart.h"


// Initialisation de la mémoire tampon du Bluetooth
static uint8_t buffer[255];
static uint8_t sommetBuffer = 0; // Sommet du buffer (0 == vide)

/*
 * Lecture de l'etat du HC-05
 */
bool_e isBluetoothPaired(void){
	return HAL_GPIO_ReadPin(STATE_GPIO, STATE_PIN);
}

/*
 * Verifie si la memoire tampon est vide
 */
bool_e isBluetoothBufferEmpty(void){
	return (sommetBuffer == 0);
}

/*
 * Retourne de caractere le plus ancien du buffer
 */
uint8_t getNextCharFromBluetoothBuffer(void){
	for(uint8_t i = 1; i <= sommetBuffer; i++){
		buffer[i-1] = buffer[i];
	}
	sommetBuffer--;
	return buffer[0];
}

/*
 * Ajoute un nouveau caractere au buffer
 */
void addToBluetoothBuffer(uint8_t c){
	sommetBuffer++;
	buffer[sommetBuffer] = c;
}

// TEST DU BLUETOOTH avec String --> OK
void sendString(uint8_t * c, uint32_t len){
	UART_puts(UART1_ID, c, len);
}
//

// TEST DU BLUETOOTH avec Char --> OK
void sendChar(uint8_t c){
	UART_putc(UART1_ID, c);
}
//
