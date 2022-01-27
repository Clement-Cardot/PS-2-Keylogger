/**
  ******************************************************************************
  * @file    main.c
  * @author  Clement CARDOT
  * @date    11-dec-2021
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f1xx_hal.h"
#include "stm32f1_uart.h"
#include "stm32f1_sys.h"
#include "stm32f1_gpio.h"
#include "stm32f1_extit.h"
#include "macro_types.h"
#include "systick.h"
#include <stdint.h>

#include "Bluetooth.h"
#include "ps2Monitor.h"
#include "test.h"

// Definition des prototypes :

void process_ms(void);
void initBluePill(void);


/*
 * Méthode appelé à chaque ms
 */
void process_ms(void){
	if(timeout) timeout--;
}

///////////////////////////////

/*
 * Méthode qui initialise les différents composants
 */
void initBluePill(void){
	//Initialisation de la couche logicielle HAL (Hardware Abstraction Layer)
	//Cette ligne doit rester la premiï¿½re ï¿½tape de la fonction main().
	HAL_Init();

	//Initialisation de l'UART2 ï¿½ la vitesse de 115200 bauds/secondes (92kbits/s) PA2 : Tx  | PA3 : Rx.
		//Attention, les pins PA2 et PA3 ne sont pas reliï¿½es jusqu'au connecteur de la Nucleo.
		//Ces broches sont redirigï¿½es vers la sonde de dï¿½bogage, la liaison UART ï¿½tant ensuite encapsulï¿½e sur l'USB vers le PC de dï¿½veloppement.
	UART_init(UART2_ID,115200);

	//Initialisation de l'UART1 qui communique avec le module Bluetooth HC-05
	UART_init(UART1_ID,9600);

	//Initialisation de la pin PB5 qui lit la pin STATE du HC-05
	BSP_GPIO_PinCfg(GPIOB, GPIO_PIN_5, GPIO_MODE_INPUT,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW);

	//"Indique que les printf sortent vers le pï¿½riphï¿½rique UART2."
	SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

	//Initialisation du port de la led Bleu (Intégré)
	BSP_GPIO_PinCfg(LED_BLUE_GPIO, LED_BLUE_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW);

	//Initialisation du port de la led Rouge (PCB)
	BSP_GPIO_PinCfg(LED_RED_GPIO, LED_RED_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW);

	//Initialisation du port CLOCK avec interruption(PS/2)
	BSP_GPIO_PinCfg(CLOCK_GPIO, CLOCK_PIN, GPIO_MODE_IT_FALLING,GPIO_PULLUP,GPIO_SPEED_FREQ_HIGH);

	//Initialisation du port DATA (PS/2)
	BSP_GPIO_PinCfg(DATA_GPIO, DATA_PIN, GPIO_MODE_INPUT,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH);

	BSP_GPIO_PinCfg(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH);


	//On ajoute la fonction process_ms ï¿½ la liste des fonctions appelï¿½es automatiquement chaque ms par la routine d'interruption du pï¿½riphï¿½rique SYSTICK
	Systick_add_callback_function(&process_ms);
}

/*
 * Méthode principale du programme
 */
int main(void){
	initBluePill();

	// Méthode de test à executer en cas de problèmes
	// test();

	// Initialisation de l'interruption sur l'horloge
	EXTIT_set_callback(ps2Interrupt, EXTI_gpiopin_to_pin_number(CLOCK_PIN), 1);

	while(1)	//boucle de tache de fond
	{
		//Si des caracteres en attente dans le buffer et que le Bluetooth est appairé
		if(isBluetoothPaired() && !isBluetoothBufferEmpty()){
			//Les envoyer vers le smartphone
			sendChar(getNextCharFromBluetoothBuffer());
		}
	}
}
