/*
 * test.c
 *
 *  Created on: 11 janv. 2022
 *      Author: Clement CARDOT
 */
#include "Bluetooth.h"
#include "ps2Monitor.h"

void toogleBlueLED(void);
void toogleRedLED(void);
void testBluetoothSendChar(void);
void testBluetoothSendString(void);
void testIsBluetoothPaired(void);

/*
 * Methode executant tous les tests unitaires
 */
void test(void){
	toogleBlueLED();
	toogleRedLED();
	testIsBluetoothPaired();
	testBluetoothSendChar();
	testBluetoothSendString();
}

/*
 * Changement d'etat de la led bleu (Built-in)
 */
void toogleBlueLED(void){
	HAL_GPIO_TogglePin(LED_BLUE_GPIO, LED_BLUE_PIN);
}

/*
 * Changement d'etat de la led rouge (PCB)
 */
void toogleRedLED(void){
	HAL_GPIO_TogglePin(LED_RED_GPIO, LED_RED_PIN);
}

/*
 * Envoi d'un caractere en Bluetooth
 */
void testBluetoothSendChar(void){
	uint8_t c = 'T';
	sendChar(c);
}

/*
 * Envoi d'une chaine de caracteres en Bluetooth
 */
void testBluetoothSendString(void){
	uint8_t * c = "est\n";
	sendString(c, 4);
}

/*
 * Test de lecture de l'etat du HC_05
 */
void testIsBluetoothPaired(void){
	if(isBluetoothPaired()){
		toogleBlueLED();
	}
}
