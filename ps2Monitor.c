/**
  ******************************************************************************
  * @file    PS_2.c
  * @author  ClÃ©ment CARDOT
  * @date    11-dec-2021
  * @brief   PS/2 read and decode fonctions.
  ******************************************************************************
*/
#include <stdint.h>
#include "stm32f1_gpio.h"
#include "stm32f1_uart.h"
#include "Bluetooth.h"
#include "test.h"

uint8_t scancodeToChar(uint8_t scancode);

static volatile uint8_t parity = 0;
static volatile bool_e bit[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static volatile uint8_t bitcount = 0;
static volatile uint8_t data = 0x00;
static volatile uint8_t c;

static volatile bool_e shift = FALSE;
static volatile bool_e altgr = FALSE;
static volatile bool_e pressUp = FALSE;
static volatile bool_e E0_code = FALSE;

static volatile uint32_t compteurOctet = 0;
static volatile uint8_t timeout = 0;
static volatile bool_e val = 0;

/*
 * Tableau de conversion des scancodes en caractères
 * le premier élément correspond au scancode
 * le second au caractère
 * le troisième au caractère + shift
 * le quatrième au caractère + altgr
 */
const uint8_t char_table[][4] = {	// 1.scancode || 2.normal || 3.shifted || 4.altgr
	{0x0E, '²', 0x00, '²'}, // ²
	{0x16, '&', '1', '&'}, 	 // &
	{0x1E, 0xE9, '2', '~'}, // é
	{0x26, '"', '3', '#'}, // "
	{0x25, 0x27, '4', '{'}, // '
	{0x2E, '(', '5', '['}, // (
	{0x36, '-', '6', '|'}, // -
	{0x3D, 0xE8, '7', '`'}, // è
	{0x3E, '_', '8', 0x5C}, // _
	{0x46, 0xE7, '9', '^'}, // ç
	{0x45, 0xE0, '0', '@'}, // à
	{0x4E, ')', '°', ']'}, // )
	{0x55, '=', '+', '}'}, // =
	//{0x66, '\b', '\b', '\b'}, // Backspace
	{0x15, 'a', 'A', 0x00}, // a
	{0x1D, 'z', 'Z', 0x00}, // z
	{0x24, 'e', 'E', '€'}, // e   Symbole € sur 2 octets ???
	{0x2D, 'r', 'R', 0x00}, // r
	{0x2C, 't', 'T', 0x00}, // t
	{0x35, 'y', 'Y', 0x00}, // y
	{0x3C, 'u', 'U', 0x00}, // u
	{0x43, 'i', 'I', 0x00}, // i
	{0x44, 'o', 'O', 0x00}, // o
	{0x4D, 'p', 'P', 0x00}, // p
	{0x54, '^', '¨', 0x00}, // ^
	{0x5B, '$', '£', '¤'}, // $
	{0x5D, '*', 0xB5, '*'}, // *
	{0x1C, 'q', 'Q', 0x00}, // q
	{0x1B, 's', 'S', 0x00}, // s
	{0x23, 'd', 'D', 0x00}, // d
	{0x2B, 'f', 'F', 0x00}, // f
	{0x34, 'g', 'G', 0x00}, // g
	{0x33, 'h', 'H', 0x00}, // h
	{0x3B, 'j', 'J', 0x00}, // j
	{0x42, 'k', 'K', 0x00}, // k
	{0x4B, 'l', 'L', 0x00}, // l
	{0x4C, 'm', 'M', 0x00}, // m
	{0x52, 0xF9, '%', 0xF9}, // ù
	{0x5A, '\n', '\n', '\n'}, // ENTRER
	{0x61, '<', '>', '<'}, // <
	{0x1A, 'w', 'W', 0x00}, // x
	{0x22, 'x', 'X', 0x00}, // c
	{0x21, 'c', 'C', 0x00}, // c
	{0x2A, 'v', 'V', 0x00}, // v
	{0x32, 'b', 'B', 0x00}, // b
	{0x31, 'n', 'N', 0x00}, // n
	{0x3A, ',', '?', ','}, // ,
	{0x41, ';', '.', ';'}, // ;
	{0x49, ':', '/', ':'}, // :
	{0x4A, '!', '§', '!'}, // !
	{0x29, ' ', ' ', ' '}, // Espace
	{0x70, '0', 0x00, 0x00}, // 0 NUMPAD
	{0x69, '1', 0x00, 0x00}, // 1 NUMPAD
	{0x72, '2', 0x00, 0x00}, // 2 NUMPAD
	{0x7A, '3', 0x00, 0x00}, // 3 NUMPAD
	{0x6B, '4', 0x00, 0x00}, // 4 NUMPAD
	{0x73, '5', 0x00, 0x00}, // 5 NUMPAD
	{0x74, '6', 0x00, 0x00}, // 6 NUMPAD
	{0x6C, '7', 0x00, 0x00}, // 7 NUMPAD
	{0x75, '8', 0x00, 0x00}, // 8 NUMPAD
	{0x7D, '9', 0x00, 0x00}, // 9 NUMPAD
	{0x71, '.', 0x00, 0x00}, // . NUMPAD
	{0x7C, '*', 0x00, 0x00}, // * NUMPAD
	{0x7B, '-', 0x00, 0x00}, // - NUMPAD
	{0x79, '+', 0x00, 0x00}, // + NUMPAD
	{0x00, 0x00, 0x00, 0x00} // Fin de liste
};

/*
 * Méthode de lecture des données PS/2
 * Fonction appelé à chaque interruption sur le GPIO PB3 (Clock)
 * Cette fonction va enregistrer chaque bit passant dans le canal Data
 * et dès qu'un octet sera lu en entier, un scancode sera généré puis converti en caractère
 * et enfin envoyé vers le module bluetooth
 */
void ps2Interrupt(void){

	static volatile bool_e start = 0;
	toogleBlueLED();

		if(start == 0 && HAL_GPIO_ReadPin(DATA_GPIO, DATA_PIN) == 0){  // Si on détecte un bit de start
			start = 1;
			timeout = 2;
			compteurOctet++;
		}

		if(timeout == 0){		// Timeout si la lecture dure plus de 2ms
			pressUp = FALSE;
			bitcount = 0;
			start = 0;
		}

		if(start){
			switch(bitcount){
				case 0:		// Bit de Start du premier octet
					parity = 0;
					bitcount ++;
					break;
				case 1:		// 8 Bits de l'octet
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					val = HAL_GPIO_ReadPin(DATA_GPIO, DATA_PIN);
					parity += val;
					bit[bitcount-1] = val;
					bitcount ++;
					break;
				case 9:		// Bit de parité
					parity &= 1;
					if(parity == HAL_GPIO_ReadPin(DATA_GPIO, DATA_PIN)){
						// Parity Error (Ici ignore)
					}
					else{
						data = bit[7] << 7 | bit[6] << 6 | bit[5] << 5 | bit[4] << 4 | bit[3] << 3 | bit[2] << 2 | bit[1] << 1 | bit[0];
					}
					bitcount ++;
					break;
				case 10:	// Bit de Stop
					// Remise à zéro des variables
					start = 0;
					bitcount = 0;
					parity = 0;

					if(pressUp){ // Si un précédent octet indique que la touche a été relaché

						if(E0_code){ // Si un précédent octet vaut E0 (code permettant l'ajout de nouvelles touches : altgr, NUMPAD, Fleches...)

							switch (data){
								case 0x11:
									altgr = FALSE;
									break;
								case 0x5A: // Enter NUMPAD
									//addToBluetoothBuffer((uint8_t)'\n');
									break;
								case 0x4A:
									//addToBluetoothBuffer((uint8_t)'/');
									break;
							}
						}

						else{ // S'il s'agit d'une touche standard

							switch (data){
								case 0x58:  // Verr MAJ
									shift = !shift;
									break;
								case 0x59:  // shift droit
								case 0x12:	// shift gauche
									shift = FALSE;
									break;
								case 0x05: // F1
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'1');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x06: // F2
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'2');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x04: // F3
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'3');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x0C: // F4
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'4');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x03: // F5
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'5');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x0B: // F6
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'6');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x83: // F7
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'7');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x0A: // F8
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'8');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x01: // F9
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'9');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x09: // F10
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'1');
									addToBluetoothBuffer((uint8_t)'0');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x78: // F11
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'1');
									addToBluetoothBuffer((uint8_t)'1');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x07: // F12
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'F');
									addToBluetoothBuffer((uint8_t)'1');
									addToBluetoothBuffer((uint8_t)'2');
									addToBluetoothBuffer((uint8_t)']');
									break;
								case 0x66:
									addToBluetoothBuffer((uint8_t)'[');
									addToBluetoothBuffer((uint8_t)'/');
									addToBluetoothBuffer((uint8_t)'b');
									addToBluetoothBuffer((uint8_t)']');
									break;
								default:
									// Envoyer le caractère au buffer (en passant par la la fonction scancodeToChar)
									toogleRedLED();
									c = scancodeToChar(data);
									addToBluetoothBuffer(c);
									break;
							}
						}

						//Remise à zéro des variables
						pressUp = FALSE;
						E0_code = FALSE;

					}

					else{ //Si la touche est toujours enfoncé
						if(E0_code){
							switch(data){
								case 0x11:
									altgr = TRUE;
									break;
							}
							E0_code = FALSE;

						}
						else{
							switch (data){
								case 0xF0: // Indique que la touche vient d'être relaché
									pressUp = TRUE;
									break;
								case 0x59:  // shift droit
								case 0x12:	// shift gauche
									shift = TRUE;
									break;
								case 0xE0: // E0 code : indique une touche non standard
									E0_code = TRUE;
									break;
							}
						}
					}
					data = 0x00;
					break;
				default : // Si bitcount en dehors des valeurs prévu : Remise à zéro
					bitcount = 0;
					start = 0;
					data = 0x00;
					break;
				}
		}
}

/*
 * Fonction permettant de convertir les scancodes en caractères
 * paramètre :
 *  - uint8_t scancode : le scancode à convertir
 *
 * retourne un caractère dans un uint8_t
 */
uint8_t scancodeToChar(uint8_t scancode){
	uint8_t i;
	for(i = 0; scancode != char_table[i][0]; i++){ // Cherche la ligne du tableau correspondante
		if(char_table[i][0] == 0x00) return 0x00;
	}
	if(scancode == char_table[i][0]){			   // Ressort l'élement de la ligne correspondant en fonction des touches appuiées en parallèle (shift, altgr ou rien)
		if(shift)	   return char_table[i][2];
		else if(altgr) return char_table[i][3];
		else 		   return char_table[i][1];
	}
	return 0x00; //Si aucun élement ne correspond retourne NULL
}

