#include "Globals.h"
#include "Music.h"
#include "Switch.h"
#include "Output.h"
#include "OLEDdraw.h"
#include "rit128x96x4.h"
#include "Timer0A.h"
#include "UART2.h"
#include "Xbee.h"
#include "music.h"
#include <stdio.h>
int playing = 0;

//debug code
void freeMode(void);
int main(void){ 
	volatile unsigned long delay;
	unsigned char * data = 0;
	char buttons = 0;
	int watch = 0;
	char printBuffer[50];
	char numCorrect[50];
	char numError[50];
	char numIncorrect[50];
	
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC;  // activate port C
	delay = SYSCTL_RCGC2_R;          // allow time to finish activating
	GPIO_PORTC_DIR_R |= 0x20;        // make PC5 out (PC5 built-in LED)
	GPIO_PORTC_DEN_R |= 0x20;        // enable digital I/O on PC5
	
	//Zigbee initalization
	UART_Init();              // initialize UART
	Xbee_Init(12);
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER0;// activate timer0
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER1;// activate timer0
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER2;// activate timer0
	
	DAC_Init(2048);
	//Timer0A_Init();       // initialize timer0A (~20,000 Hz)
	//Timer1A_Init();       // initialize timer0A (~20,000 Hz)
	
	FrequencyTimersInit();
	MIDIParserInit();
	OLEDTimerInit();
	
	//Switch_Init();
	Output_Init();
	EnableInterrupts();
	
	
	RIT128x96x4DisplayOn();
	//RIT128x96x4_ClearImage();
	
	//RIT128x96x4_Line(0,0,50,50,15);
	//drawCircle(50,50,10)
	RIT128x96x4_ShowImage();
	printf("Hit Button 0 For Free Mode\nHit Button 1 To Play Guitar Hero!\n");
	
	/*
	while(data==0){
		data= receiveData();
		buttons = data[0] & 0x0F;
		if(buttons == 1) {
			playMode = 1;
			freeMode();
		}
		else if(buttons == 2){
			playMode = 0;
			break;
		}
	}*/
	
	//frequency timers
	game_running = 1;
	TIMER0_CTL_R |= TIMER_CTL_TAEN;
	TIMER1_CTL_R |= TIMER_CTL_TAEN;
	TIMER2_CTL_R |= TIMER_CTL_TAEN;
	
	TIMER0_CTL_R |= TIMER_CTL_TBEN;
	TIMER1_CTL_R |= TIMER_CTL_TBEN;
		
  while(game_running == 1){
		//if (playMode ==1) freeMode();
		RIT128x96x4_ClearImage();
		sprintf(printBuffer, "Errors: %d\nCorrects: %d", errors, corrects);
		//printf("Corrects: %d\n", corrects);
		RIT128x96x4StringDraw(printBuffer, 0, 0, 10);
		BufferDraw();
		RIT128x96x4_ShowImage();
  }
	
		TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
		TIMER0_CTL_R &= ~TIMER_CTL_TBEN;
		TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
		TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
		TIMER1_CTL_R &= ~TIMER_CTL_TBEN;
		TIMER2_CTL_R &= ~TIMER_CTL_TBEN;
		game_running = 0;
	
	RIT128x96x4_ClearImage();
	RIT128x96x4_ShowImage();
	sprintf(numCorrect, "Correct Notes: %d", corrects);
	sprintf(numIncorrect, "Missed Notes: %d", SONG_SIZE - corrects);
	sprintf(numError, "Incorrect Notes: %d", errors);
	RIT128x96x4StringDraw("Well Played!", 20, 20, 15);
	RIT128x96x4StringDraw(numCorrect, 20, 40, 15);
	RIT128x96x4StringDraw(numIncorrect, 20, 60, 15);
	RIT128x96x4StringDraw(numError, 20, 80, 15);
	
	while(1){
		
	}
	
}

void freeMode(void){ 		//if hit, play note corresponding to given hit pattern
	  long notes [4] = {C, D, E, F};
		unsigned char * data = 0;
		char buttons = 0;
		printf("start free mode\n");

		TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
		TIMER0_CTL_R &= ~TIMER_CTL_TBEN;
		TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
		TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
		TIMER1_CTL_R &= ~TIMER_CTL_TBEN;
		TIMER2_CTL_R &= ~TIMER_CTL_TBEN;
		
	while(playMode == 1){
		while (data ==0) 
			data = receiveData(); // poll zigbee queue
		
		buttons = (data[0] & 0x0F);
		//printf("hit: %d                \n", buttons);
		RIT128x96x4_ClearImage();
	
		if (buttons & 0x01)
			drawCircle(32*1-16, 44, 128/8);
		if (buttons & 0x02)
			drawCircle(32*2-16, 44, 128/8);
		if (buttons & 0x04)
			drawCircle(32*3-16, 44, 128/8);
		if (buttons & 0x08)
			drawCircle(32*4-16, 44, 128/8);
		RIT128x96x4_ShowImage();

		switch(buttons){
			case 1: FMNote1 = FMNote2 = notes[0]; break;
			case 2: FMNote1 = FMNote2 = notes[1]; break;
			case 4: FMNote1 = FMNote2 = notes[2]; break;
			case 8: FMNote1 = FMNote2 = notes[3]; break;
			case 3: FMNote1 = FMNote2 = E; break;
			case 5: FMNote1 = FMNote2 = F; break;
			case 6: FMNote1 = FMNote2 = EF; break;
		  case 9: FMNote1 = FMNote2 = G; break;
			case 10: FMNote1 = FMNote2 = GF; break;
			case 12: FMNote1 = FMNote2 = AF; break;
			default: FMNote1 = FMNote2 = DF; break;
		}
		if (buttons > 0){
			FMcountNoteLength =0;
			FMcyclesLeft1 = 0;
			FMcyclesLeft2 = 0;
			TIMER0_CTL_R |= TIMER_CTL_TAEN;
			TIMER1_CTL_R |= TIMER_CTL_TAEN;
			//start playing
		}
		data = 0;
	}
}
