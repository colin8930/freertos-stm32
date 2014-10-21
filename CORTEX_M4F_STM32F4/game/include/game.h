#ifndef GAME_H
#define GAME_H

void
gryo_init();

void
gryo_update();

void
BallReset();

void
GAME_EventHandler1();

void
GAME_EventHandler2();


void
GAME_EventHandler2();


int
UART_EventHandler1();

void
GAME_Update();

void
GAME_Render();

void 
RCC_Configuration();

void 
GPIO_Configuration();


void USART1_puts(char*);

#endif
