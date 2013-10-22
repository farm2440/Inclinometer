//----------------------------------------------------------------------------
// C main line
//----------------------------------------------------------------------------
#include <m8c.h>        // part specific constants and macros
#include "PSoCAPI.h"    // PSoC API definitions for all User Modules
#include <math.h>
#include <stdio.h>

WORD	spiTalk(BYTE cmd);
void 	delay(unsigned int t);

//SPI интерфейс. Т.К. Данните за ъгъла са 11 битови не се ползва готов SPI модул
#define MOSI	0x80  //P0.7
#define MISO	0x20  //P0.5
#define SCK		0x08  //P0.3
#define CSB		0x02  //P0.1

#define BTN		0x80  //P1.7
//Команди на SCA100T
#define MEAS	0x00	//Режим на измерване
#define RWTR	0x08	//Чете температура
#define STX		0x0E
#define STY		0x0F	//Самотест X/Y
#define	RDAX	0x10 
#define RDAY	0x11	//Четене на X/Y

BYTE	shadow_P0;

char buff[64];	
float ax,ay,offx,offy,tmpf; //променливите 
WORD dx,dy;
WORD dOffX, dOffY; ////Спрямо тези променливи става проверката за постигната вертикалност.
char dcnt; //брояч за изобразяване на стойностите през няколко
char cnt, n, p1, t; //брояч за следене на бутона
char fAdjustX, fAdjustY; //Тeзи флагове се вдигат при кратко натискане на бутона и остават вдигнати до постигане на 
						//верикално положение или до повторно кратко натискане

void main(void)
{
	// M8C_EnableGInt ; // Uncomment this line to enable Global Interrupts
	shadow_P0 = CSB; //Такта е първоначално в 1, SCK и MOSI са 0
	LCD_Start();
	LCD_Init();	
	
	LCD_Position(0,0);	
	LCD_PrCString("STIV Engineering");
	LCD_Position(1,0);	
	LCD_PrCString(" v.1.0.37       ");	
	
	delay(2000);
	
	LCD_Position(0,0);	
	LCD_PrCString("                ");
	LCD_Position(1,0);	
	LCD_PrCString("                ");
	
	//зареждане на офсета от FLASH-a. 
	//Офсета е запазен при процедурата по калибриране.
	//offx=1024;
	//offy=1024;
	//Четене от EEPROM
	EEPROM_Start();
	EEPROM_E2Read(0, buff, 64);
	EEPROM_Stop();
	dOffX = ((WORD)buff[0])<<8;
	dOffX += buff[1];
	dOffY = ((WORD)buff[2])<<8;
	dOffY += buff[3];
	offx = dOffX;
	offy = dOffY;
	
	PRT0DR = shadow_P0;
		
	dcnt = 1;
	
	while(1)
	{
readSPI:	
		dx = spiTalk(RDAX);
		tmpf = (float)(dx-offx);
		tmpf /= 1638;
		ax=asin(tmpf)*180/_PI;
		
		delay(15);
				
		dy = spiTalk(RDAY);
		tmpf = (float)(dy-offy);
		tmpf /= 1638;
		ay=asin(tmpf)*180/_PI;
		
		delay(15);
		
		/*	Due to code size requirement, the printf/scanf functions are supported in the following three forms.

			1. Basic with no long / floating point, or modifier,
			2. Support for long , and
			3. Support for floating point.

			By default the option 1 is supported. To use option 2 or 3 the following changes are required in the compiler settings.

			Support for float:
			1. Open local.mk file from "Projects" tab in PSoC Designer. If local.mk file is not already available, create a text file called local.mk in the project directory.
			2. At the bottom of local.mk, add CODECOMPRESSOR:=$(CODECOMPRESSOR) -lfpm8c  (this will link the libfpm8c.a, which contains the floating point capability of printf/scanf with modifier support)
			3. Save local.mk file and build the project.

			More code and data/stack space are needed for the fully featured versions (for the support of long and float).

			Support for long:
			1. Open local.mk file from "Projects" tab in PSoC Designer. If local.mk file does not already exist, create a text file called local.mk in the project directory.
			2. At the bottom of local.mk, add CODECOMPRESSOR:=$(CODECOMPRESSOR) -llpm8c  (this will link the liblpm8c.a , which contains the long data type capability of printf/scanf)
			3. Save local.mk file and build the project.
		*/
		p1=PRT1DR & 0x80;
		if(p1 != BTN)
		{//бутона е натиснат
			//1.ако е натиснат и отпуснат за време под 1 секунда се влиза/излиза в режим на изправяне
			//2.ако след първата секунда не е отпуснат се изписва надпис "CALIBRATE....." като точките се изписват по 1 в секунда.
			//  ако в този период бутона се отпусне нищо не се случва
			//3. След като се изпишат и петте точки се чака отпускане на бутона и при отускане се чете стойност за X и Y и се записват за офсет.
			LCD_Position(0,0);	
			LCD_PrCString("    >>>0<<<     ");
			LCD_Position(1,0);	
			LCD_PrCString("                ");						
			
			//1.
		  	for(cnt=150 ; cnt !=0 ; cnt --) 
			{
				delay(10);
				if(PRT1DR & BTN) 
				{//бутона е отпуснат - кратко натискане. Вдига се флага за режим на изправяне ако и двата са били свалени
					if((fAdjustX==0) && (fAdjustY==0))
					{//Влиза в режим на изправяне
						dcnt=1;
						fAdjustX=1;
						fAdjustY=1;
					}
					else
					{//Излиза се от режим на изправяне
						fAdjustX=0;
						fAdjustY=0;
					}
					LCD_Position(0,0);	
					LCD_PrCString("                ");
					goto displayValues;
				}
			}
			//2.
			LCD_Position(0,0);	
			LCD_PrCString(" CALIBRATE      ");
			LCD_Position(0,10);
			for(n=6 ; n!=0 ; n--)
			{
				for(cnt=100 ; cnt !=0 ; cnt --) 
				{
					delay(10);
					if(PRT1DR & BTN) 
					{
						LCD_Position(0,0);	
						LCD_PrCString("                ");
						goto displayValues;
					}
				}
				LCD_WriteData('.');
			}
			//3.
			dx = spiTalk(RDAX);
			dy = spiTalk(RDAY);
			offx=dx;
			offy=dy;
			LCD_Position(0,0);	
			LCD_PrCString("     offset:    ");
			LCD_Position(1,0);	
			csprintf(buff," x=%4d y=%4d ",dx,dy);
			LCD_PrString(buff);
			//прочитане на температурата от SCA100T. Ползава се за запис в EEPROM
			t = (char)spiTalk(RWTR);
			tmpf = (197.0f - (float)t) * 0.92f;
			if(tmpf<0.0f) tmpf=0.0f;
			t = (char) tmpf;
			//ZA PROBA--------------
			LCD_Position(0,0);	
			LCD_PrCString("                ");
			csprintf(buff,"t=%d",t);
			LCD_Position(0,0);	
			LCD_PrString(buff);		
  			//-------------------------
			//Запис в EEPROM
			buff[0] =(char)((dx & 0xff00)>>8);
			buff[1] =(char) (dx & 0x00ff);
			buff[2] =(char)((dy & 0xff00)>>8);
			buff[3] =(char) (dy & 0x00ff);
			dOffX = dx;
			dOffY = dy;//Спрямо тези променливи става проверката за постигната вертикалност.
			EEPROM_Start();
			n = EEPROM_bE2Write(0, buff, 64, t);
			EEPROM_Stop();
			if(n==-1) 
			{
				LCD_Position(0,0);	
				LCD_PrCString("ERROR:          ");
				LCD_Position(1,0);	
				LCD_PrCString("Write fail!     ");
				delay(4000);
			}
			if(n==-2) 
			{
				LCD_Position(0,0);	
				LCD_PrCString("ERROR:          ");
				LCD_Position(1,0);	
				LCD_PrCString("Stack overflow! ");
				delay(4000);
			}
			//Чакам бутона да бъде отпуснат
			do { p1=PRT1DR & 0x80; }
			while(p1 != BTN);
			LCD_Position(0,0);	
			LCD_PrCString("                ");
			LCD_Position(1,0);	
			LCD_PrCString("                ");
		}	
		
displayValues:		
		dcnt--;
		if(dcnt==0)
		{
			dcnt = 10;
			csprintf(buff,"x=%6.2f",ax);
			LCD_Position(0,0);	
			LCD_PrString(buff);	
	
			csprintf(buff,"y=%6.2f",ay);
			LCD_Position(1,0);	
			LCD_PrString(buff);	
		}
		//Проверка за вертикалност
		if(	(fAdjustX==0) && (fAdjustY==0))
		{//Не сме в режим на изправяне - всички символи след ъгъла се изтриват
			LCD_Position(0,9);
			LCD_PrCString("        ");
			LCD_Position(1,9);
			LCD_PrCString("        ");
		}
		else
		{	//Проверка по Х
			LCD_Position(0,9);
			if(dx>(dOffX+29))
			{
				fAdjustX=1;
				LCD_PrCString("   0<<  ");
			}
			else
			{
				if(dx<(dOffX-29))
				{
					fAdjustX=1;
					LCD_PrCString(" >>0    ");
				}
				else 
				{//Постигната е вертикалност по Х
					fAdjustX=0;
					LCD_PrCString(">>>0<<< ");
					if(fAdjustY==0) delay(1000); //Задържа 1 секунда в момента когато има пълна вертикалност
				}
			}
			//Проверка по Y
			LCD_Position(1,9);
			if(dy>(dOffY+29))
			{
				fAdjustY=1;
				LCD_PrCString("   0<<  ");
			}
			else
			{				
				if(dy<(dOffY-29))
				{
					fAdjustY=1;
					LCD_PrCString(" >>0    ");
				}
				else 
				{//Постигната е вертикалност по Y
					fAdjustY=0;
					LCD_PrCString(">>>0<<< ");
					if(fAdjustX==0) delay(1000); //Задържа 1 секунда в момента когато има пълна вертикалност
				}
			}
		}	
	}
}	
//--------------------------------------------------------------
WORD	spiTalk(BYTE cmd)
{//Изпраща команда по SPI към инклиномера. В зависимост от командата връща данни или 0
	BYTE i;
	WORD data;
	BYTE tmp = cmd;
	
	shadow_P0 &= (~CSB);//нулира CS		
	PRT0DR = shadow_P0;
	//KOMAHДА
	for(i=0 ; i<8 ; i++)
	{
		if(tmp & 0x80) shadow_P0 |= MOSI;
		else shadow_P0 &= (~MOSI);
		PRT0DR = shadow_P0; //Данната е изведена
		
		shadow_P0 |= SCK; 
		PRT0DR = shadow_P0;//преден фронт на такта
		
		shadow_P0 &= (~SCK); 
		PRT0DR = shadow_P0;//заден фронт на такта
		
	    tmp <<= 1;
	}	
	//ДАННИ	
	shadow_P0 &= (~MOSI);
	PRT0DR = shadow_P0; 
	
	switch(cmd)
	{
		case RDAX:
		case RDAY:
			i=11;
			break;
		case RWTR:
			i=8;
			break;
		default:
			i=0;
			shadow_P0 |= CSB;//set CS		
			PRT0DR = shadow_P0;
			return 0;
	}
	
	data = 0;
	do
	{
		shadow_P0 |= SCK; 
		PRT0DR = shadow_P0;//преден фронт на такта
		
		data <<= 1;
		if(PRT0DR & MISO) data |= 1;				
		
		shadow_P0 &= (~SCK); 
		PRT0DR = shadow_P0;//заден фронт на такта

		i--;
	}while(i);

	shadow_P0 |= CSB;//set CS		
	PRT0DR = shadow_P0;
	return	data;
}
//----------------------------------------------
void delay(unsigned int t)
{//Отработва закъснение от t ms.
	unsigned int i,j;
	for(j=t ; j!=0 ; j--)
	{		
		for(i=59; i!=0 ; i--) {} 
	}
}