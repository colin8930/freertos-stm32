#include "game.h"
#include "main.h"
#include "stm32f4xx.h"
#include "shell.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_l3gd20.h"
#include "FreeRTOS.h"
#include "task.h"
#include <r3dfb.h>
#include <r3d.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "semphr.h"

// vertex format
typedef struct {
	uint16_t x, y, z;
	uint8_t nx, ny, nz;
	uint8_t u, v;
} vertex_t;

typedef struct {
	int LifePoint;	
	int16_t centerX ;
	int16_t centerY;
	
} me;


typedef struct {
	int LifePoint;	
	int gun[3];
	int16_t centerX ;
	int16_t centerY ;	
	int gunCount;
} boss;

typedef struct {
	int16_t ballX;
	int16_t ballY;
	int16_t ballVX;
	int16_t ballVY;
	uint8_t ballIsRun;
} bullet;

boss boss1[2];
me me1[2];
int shoot=0;

int layer_state=0;

const int body[2][3] = {{1,1,1},{0,1,0}};

//boss

int16_t bossVX=5;
#define MAX_BOSS_BULLET 3
#define MAX_MY_BULLET 3

typedef struct {
	bullet bullet [3];
} bullets;


bullets boss_bullet[2];
bullets  my_bullet[2];

int axes[3] = {0};
float time = 0.0f, frametime = 0.0f;
int standby=0;







uint32_t layer_next=LCD_BACKGROUND_LAYER;
uint8_t SHOW=0xFF;
uint8_t HIDE=0x00;

//Mode
uint8_t demoMode = 0;

//layer
int layer = 1;

//input char
char arrowKey='\n';

extern volatile xQueueHandle  t_queue;
extern int states[];

//gyro init
void gryo_init()
{

	L3GD20_InitTypeDef L3GD20_InitStructure;
	L3GD20_InitStructure.Power_Mode = L3GD20_MODE_ACTIVE;
	L3GD20_InitStructure.Output_DataRate = L3GD20_OUTPUT_DATARATE_1;
	L3GD20_InitStructure.Axes_Enable = L3GD20_AXES_ENABLE;
	L3GD20_InitStructure.Band_Width = L3GD20_BANDWIDTH_4;
	L3GD20_InitStructure.BlockData_Update = L3GD20_BlockDataUpdate_Continous;
	L3GD20_InitStructure.Endianness = L3GD20_BLE_LSB;
	L3GD20_InitStructure.Full_Scale = L3GD20_FULLSCALE_250;
	L3GD20_Init(&L3GD20_InitStructure);

	L3GD20_FilterConfigTypeDef L3GD20_FilterStructure;
	L3GD20_FilterStructure.HighPassFilter_Mode_Selection = L3GD20_HPM_NORMAL_MODE_RES;
	L3GD20_FilterStructure.HighPassFilter_CutOff_Frequency = L3GD20_HPFCF_0;
	L3GD20_FilterConfig(&L3GD20_FilterStructure);
	L3GD20_FilterCmd(L3GD20_HIGHPASSFILTER_ENABLE);
}


int gyro_Mapping(int axis)
{

		return axis*0.1;

}

//gyro updata
void gryo_update()
{
	uint8_t tmp[6] = {0};
	int16_t a[3] = {0};
	uint8_t tmpreg = 0;

	L3GD20_Read(&tmpreg, L3GD20_CTRL_REG4_ADDR, 1);
	L3GD20_Read(tmp, L3GD20_OUT_X_L_ADDR, 6);

	/* check in the control register 4 the data alignment (Big Endian or Little Endian)*/
	if (!(tmpreg & 0x40)) {
		for (int i = 0; i < 3; i++)
			a[i] = (int16_t)(((uint16_t)tmp[2 * i + 1] << 8) | (uint16_t)tmp[2 * i]);
	} else {
		for (int i = 0; i < 3; i++)
			a[i] = (int16_t)(((uint16_t)tmp[2 * i] << 8) | (uint16_t)tmp[2 * i + 1]);
	}

	

		axes[1] = a[1] / 114.285f;  //only need axes[1]
	
 // y: axes[1]

}

void 
boss_init()
{

	for(int i=0; i<2; i++){
		
		boss1[i].LifePoint=9;
		boss1[i].gun[0]=1;
		boss1[i].gun[1]=1;
		boss1[i].gun[2]=1;
		boss1[i].gunCount=3;
		boss1[i].centerX=LCD_PIXEL_WIDTH/2;
		boss1[i].centerY=LCD_PIXEL_HEIGHT - 20;

	}
	
	
}

void boss_bullet_init()
{

	if(boss1[0].gun[0]==1)
		boss_bullet[0].bullet[0].ballX = boss1[0].centerX-10 ;
		boss_bullet[0].bullet[0].ballY = boss1[0].centerY-10*3;
		boss_bullet[0].bullet[0].ballVX = 0;
		boss_bullet[0].bullet[0].ballVY = 5;

	if(boss1[0].gun[1]==1)
		boss_bullet[0].bullet[1].ballX = boss1[0].centerX ;
		boss_bullet[0].bullet[1].ballY = boss1[0].centerY-10*4;
		boss_bullet[0].bullet[1].ballVX = 0;
		boss_bullet[0].bullet[1].ballVY = 5;

	if(boss1[0].gun[2]==1)
		boss_bullet[0].bullet[2].ballX = boss1[0].centerX+10 ;
		boss_bullet[0].bullet[2].ballY = boss1[0].centerY-10*3;
		boss_bullet[0].bullet[2].ballVX = 0;
		boss_bullet[0].bullet[2].ballVY = 5;


}

void my_bullet_init()
{
	standby=3;
	for(int i=0; i<3; i++){
		my_bullet[0].bullet[i].ballX = me1[0].centerX;
		my_bullet[0].bullet[i].ballY = me1[0].centerY;
		my_bullet[0].bullet[i].ballIsRun=0;
	}

}

void 
me_init()
{
	for(int i=0; i<2; i++){

		me1[i].LifePoint=9;
		me1[i].centerX= LCD_PIXEL_WIDTH/2;
		me1[i].centerY= 20;

	}
	

}

void
draw_me(me me2)
{
		for(int i=0; i<2;i++)
		{
			for(int j=0;j<3;j++)
			{
				if(body[i][j]==1) 
					LCD_DrawFullRect( me2.centerX-10*(j-2), me2.centerY+10*i, 10, 10 );
			}
		}

}


void
draw_boss(boss boss2)
{
		for(int i=0; i<2;i++)
		{
			for(int j=0;j<3;j++)
			{
				if(body[i][j]==1) 
					LCD_DrawFullRect( boss2.centerX-10*(j-1), boss2.centerY-10*i, 10, 10 );
			}
		}
	
			
}

void
draw_gun(boss boss2)
{

			if(boss2.gun[0]==1)
			{
				LCD_DrawFullRect( boss2.centerX-10, boss2.centerY-10, 10, 10 );
			}
			if(boss2.gun[1]==1)
			{
				LCD_DrawFullRect( boss2.centerX, boss2.centerY-20, 10, 10 );
			}
			if(boss2.gun[2]==1)
			{
				LCD_DrawFullRect( boss2.centerX+10, boss2.centerY-10, 10, 10 );
			}

}


void
draw_bossbullet(bullets bulletsin)
{
	for(int i=0; i<3; i++){
		if(boss1[0].gun[i]==1)
		LCD_DrawFullRect(  bulletsin.bullet[i].ballX, bulletsin.bullet[i].ballY, 10, 10 );
	}

}

void
draw_mybullet(bullets bulletsin)
{
	
	for(int i=0; i<3;i++)
	{
		
		if(bulletsin.bullet[i].ballIsRun==1) 
			LCD_DrawFullRect( bulletsin.bullet[i].ballX, bulletsin.bullet[i].ballY, 10, 10 );		
	}

}


void
GAME_Reset()
{
	boss_init();
	me_init();
	
}

void
GAME_EventHandler1()
{
	gryo_update();
}
void
GAME_EventHandler2()
{
	const portTickType xDelay = 100;
	if(STM_EVAL_PBGetState(BUTTON_USER))
	{	
		shoot=1;	
		vTaskDelay(100);		
	}
}

void
GAME_Update()
{
	
	//char str[5];
	if( demoMode == 0 ){

		
		me1[0].centerX  += gyro_Mapping(axes[1])*1.5;
		if(shoot==1){

			if(standby>0){
			for(int i=0; i<3; i++) {
				if(my_bullet[0].bullet[i].ballIsRun==0) {					
					my_bullet[0].bullet[i].ballIsRun=1;
					standby--;
					shoot=0;
					
					break;
				}
				
			}			
		}

		}

		if( me1[0].centerX  <= 30 )
			me1[0].centerX = 30;
		else if( me1[0].centerX  >= LCD_PIXEL_WIDTH -30)
			me1[0].centerX  = LCD_PIXEL_WIDTH - 30;



		boss1[0].centerX+=bossVX;
		if(boss1[0].centerX<=20){
			boss1[0].centerX=20;
			bossVX*=-1;
		}
		else if(boss1[0].centerX>=LCD_PIXEL_WIDTH -20){

			boss1[0].centerX=LCD_PIXEL_WIDTH -20;
			bossVX*=-1;
		}
			
		
		
			//draw bullet black

			
			
			//boss bullet
			//move bullet
			for(int i=0; i<3; i++)
			{

				if(boss1[0].gun[i]==1)
				{
					if(boss_bullet[0].bullet[i].ballY<=0) {
						boss_bullet_init();
						break;
					}
					
					else{
						boss_bullet[0].bullet[i].ballY-=5;
					}
				}

			}
			//my bullet
			for(int i=0; i<3; i++)
			{

				if(my_bullet[0].bullet[i].ballIsRun==1)
				{
					if(my_bullet[0].bullet[i].ballY>=LCD_PIXEL_HEIGHT - 20) {
						standby++;
						my_bullet[0].bullet[i].ballIsRun=0;
						my_bullet[0].bullet[i].ballX = me1[0].centerX;
						my_bullet[0].bullet[i].ballY = me1[0].centerY;						
						continue;
					}
					
					else{
						my_bullet[0].bullet[i].ballY+=5;
					}
				}

			}
			

			//PONG! me
			for(int i=0; i<3; i++)
			{

				if(boss1[0].gun[i]==1)
				{
					if(boss_bullet[0].bullet[i].ballY<=me1[0].centerY){

						if(boss_bullet[0].bullet[i].ballX<=me1[0].centerX+10&&boss_bullet[0].bullet[i].ballX>=me1[0].centerX-10) {
							me1[0].LifePoint--;
						}
						else{

							boss_bullet_init();
						}
					}
							
				}

			}
			
			//PONG! boss
			for(int i=0; i<3; i++){
				if(my_bullet[0].bullet[i].ballIsRun){
					if(my_bullet[0].bullet[i].ballY>=boss1[0].centerY)
					{
						if(my_bullet[0].bullet[i].ballX<=boss1[0].centerX+20&&my_bullet[0].bullet[i].ballX<=boss1[0].centerX-20){
							if(boss1[0].gunCount==3){
								LCD_SetLayer(LCD_BACKGROUND_LAYER);
								LCD_SetTextColor( LCD_COLOR_BLACK );
								draw_bossbullet(boss_bullet[1]);
								LCD_SetLayer(LCD_FOREGROUND_LAYER);
								boss1[0].gun[2]=0;
								boss1[1].gun[2]=0;
								boss1[0].gunCount--;
							}
							else if(boss1[0].gunCount==2){
								LCD_SetLayer(LCD_BACKGROUND_LAYER);
								LCD_SetTextColor( LCD_COLOR_BLACK );
								draw_bossbullet(boss_bullet[1]);
								LCD_SetLayer(LCD_FOREGROUND_LAYER);
								boss1[0].gun[0]=0;
								boss1[1].gun[0]=0;
								boss1[0].gunCount--;
							}
							boss1[0].LifePoint--;
							standby++;
							my_bullet[0].bullet[i].ballIsRun=0;
							my_bullet[0].bullet[i].ballX = me1[0].centerX;
							my_bullet[0].bullet[i].ballY = me1[0].centerY;		
						}
							
					}

				}


			}

		

		
	}
}

static char* itoa(int value, char* result, int base)
{
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}
	char *ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while (value);

	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}


void
GAME_Render()
{
	
	LCD_SetTextColor( LCD_COLOR_WHITE );
	draw_me(me1[0]);
	draw_boss(boss1[0]);
	
	LCD_SetTextColor( LCD_COLOR_RED );
	draw_bossbullet(boss_bullet[0]);
	draw_gun(boss1[0]);

	LCD_SetTextColor( LCD_COLOR_BLUE );
	draw_mybullet(my_bullet[0]);

	LTDC_LayerAlpha(LTDC_Layer1, SHOW);
	LTDC_LayerAlpha(LTDC_Layer2, HIDE);	
	LTDC_ReloadConfig(LTDC_VBReload);
	vTaskDelay( 2.5 );


	LCD_SetLayer(LCD_BACKGROUND_LAYER);
	LCD_SetTextColor( LCD_COLOR_BLACK );
	draw_me(me1[1]);
	draw_boss(boss1[1]);
	draw_gun(boss1[1]);
	draw_mybullet(my_bullet[1]);
	draw_bossbullet(boss_bullet[1]);


	LCD_SetTextColor( LCD_COLOR_WHITE );
	draw_me(me1[0]);
	draw_boss(boss1[0]);
	
	LCD_SetTextColor( LCD_COLOR_RED );
	draw_bossbullet(boss_bullet[0]);
	draw_gun(boss1[0]);

	LCD_SetTextColor( LCD_COLOR_BLUE );
	draw_mybullet(my_bullet[0]);


	LTDC_LayerAlpha(LTDC_Layer2, SHOW);			
	LTDC_LayerAlpha(LTDC_Layer1, HIDE);
	LTDC_ReloadConfig(LTDC_VBReload);
	vTaskDelay( 2.5 );

	LCD_SetLayer(LCD_FOREGROUND_LAYER);

	LCD_SetTextColor( LCD_COLOR_BLACK );

	draw_me(me1[0]);
	draw_boss(boss1[0]);
	draw_gun(boss1[0]);
	draw_mybullet(my_bullet[0]);
	draw_bossbullet(boss_bullet[0]);
	
	boss_bullet[1]=boss_bullet[0];
	my_bullet[1]=my_bullet[0];
	me1[1]=me1[0];
	boss1[1]=boss1[0];
}


/**********************************UART***********************************************/

void RCC_Configuration(void)
{
      /* --------------------------- System Clocks Configuration -----------------*/
      /* USART1 clock enable */
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
      /* GPIOA clock enable */
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
}
 
/**************************************************************************************/
 
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*-------------------------- GPIO Configuration ----------------------------*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Connect USART pins to AF */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   // USART1_TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  // USART1_RX
}
 
/**************************************************************************************/
 
void USART1_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* USARTx configuration ------------------------------------------------------*/
    /* USARTx configured as follow:
     *  - BaudRate = 9600 baud
     *  - Word Length = 8 Bits
     *  - One Stop Bit
     *  - No parity
     *  - Hardware flow control disabled (RTS and CTS signals)
     *  - Receive and transmit enabled
     */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void USART1_puts(char* s)
{
    while(*s) {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        arrowKey=*s;
	USART_SendData(USART1, *s);
        s++;
    }
}

/**************************************************************************************/

int
UART_EventHandler1()  //for game
{
	USART_SendData(USART1, '1');
        while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
        char t = USART_ReceiveData(USART1);

	arrowKey=t;
	USART_SendData(USART1, t);
	portBASE_TYPE status;
	if(arrowKey=='p')  {		
		status = xQueueSendToBack(t_queue, &states[GAME_PAUSE],0);	
		if ( status != pdPASS)	USART1_puts("queue error");		
 	}

	if(arrowKey=='r')  {		
		status = xQueueSendToBack(t_queue, &states[GAME_RESUME],0);	
		if ( status != pdPASS)	USART1_puts("queue error");		
 	}

	if(arrowKey==ESC)  {		
		status = xQueueSendToBack(t_queue, &states[GAME_STOP],0);		
		if ( status != pdPASS)	USART1_puts("queue error");			
 	}

       if ((t == '\r')) {
       		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      		USART_SendData(USART1, t);
		arrowKey=t;
		if(arrowKey=='p')  {			
			status = xQueueSendToBack(t_queue, &states[GAME_PAUSE],0);	
			if ( status != pdPASS)	USART1_puts("queue error");				
		}

		if(arrowKey=='r')  {		
		status = xQueueSendToBack(t_queue, &states[GAME_RESUME],0);	
		if ( status != pdPASS)	USART1_puts("queue error");
 		}

		if(arrowKey==ESC)  {			
			status = xQueueSendToBack(t_queue, &states[GAME_STOP],0);	
			if ( status != pdPASS)	USART1_puts("queue error");		
 		}
 		
     }
	t = '\n';
}