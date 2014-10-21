#include "shell.h"
#include <stddef.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include "queue.h"
#include "semphr.h"
#include "main.h"


typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

extern volatile xQueueHandle t_queue;

void ps_command(int, char **);

void start_command(int, char **);

int states[] = {GAME_START,
		GAME_PAUSE,
		GAME_RESUME,
		GAME_STOP};


#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={

	MKCL(ps, "Report a snapshot of the current processes"),	
	MKCL(start, "start")

};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}


void ps_command(int n, char *argv[]){
	/*signed char buf[1024];
	vTaskList(buf);
        fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        fio_printf(1, "*******************************************\n\r");
	fio_printf(1, "%s\r\n", buf + 2);	*/
	USART1_puts("test");

}


void start_command(int n, char *argv[]){
	signed char buf[1024];
	
	USART1_puts("game start\n");
	portBASE_TYPE status;
	status = xQueueSendToBack(t_queue, &states[GAME_START],0);
	
	if ( status != pdPASS)
		USART1_puts("queue error");

}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}

