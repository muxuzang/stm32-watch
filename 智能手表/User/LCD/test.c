#include "lcd.h"
#include "delay.h"
#include "gui.h"
#include "test.h"
#include "pic.h"

//========================variable==========================//
u16 ColorTab[5]={RED,GREEN,BLUE,YELLOW,BRED};//∂®“Â—’…´ ˝◊È
//=====================end of variable======================//

/*****************************************************************************
 * @name       :void DrawTestPage(u8 *str)
 * @date       :2018-08-09 
 * @function   :Drawing test interface
 * @parameters :str:the start address of the Chinese and English strings
 * @retvalue   :None
******************************************************************************/ 
void DrawTestPage(u8 *str)
{
//ªÊ÷∆πÃ∂®¿∏up
//LCD_Clear(WHITE);
//LCD_Fill(0,0,lcddev.width,20,BLUE);
//ªÊ÷∆πÃ∂®¿∏down
//LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
//POINT_COLOR=BLUE;
Gui_StrCenter(0,12,WHITE,BLUE,str,16,0);//æ”÷–œ‘ æ
//Gui_StrCenter(0,lcddev.height-32,WHITE,BLUE,"www.lcdwiki.com",16,0);//æ”÷–œ‘ æ
//ªÊ÷∆≤‚ ‘«¯”Ú
//LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
}

/*****************************************************************************
 * @name       :void Display_ButtonUp(u16 x1,u16 y1,u16 x2,u16 y2)
 * @date       :2018-08-24 
 * @function   :Drawing a 3D button
 * @parameters :x1:the bebinning x coordinate of the button
                y1:the bebinning y coordinate of the button
								x2:the ending x coordinate of the button
								y2:the ending y coordinate of the button
 * @retvalue   :None
******************************************************************************/ 
void Display_ButtonUp(u16 x1,u16 y1,u16 x2,u16 y2)
{
	POINT_COLOR=WHITE;
	LCD_DrawLine(x1,  y1,  x2,y1); //H
	LCD_DrawLine(x1,  y1,  x1,y2); //V
	
	POINT_COLOR=GRAY1;
	LCD_DrawLine(x1+1,y2-1,x2,y2-1);  //H
	POINT_COLOR=GRAY2;
	LCD_DrawLine(x1,y2,x2,y2);  //H
	POINT_COLOR=GRAY1;
	LCD_DrawLine(x2-1,y1+1,x2-1,y2);  //V
	POINT_COLOR=GRAY2;
  LCD_DrawLine(x2,y1,x2,y2); //V
}

/*****************************************************************************
 * @name       :void menu_test(void)
 * @date       :2018-08-24 
 * @function   :Drawing a 3D menu UI
 * @parameters :None
 * @retvalue   :None
******************************************************************************/ 
void menu_test(void)
{
	LCD_Clear(GRAY0);
	
	Display_ButtonUp((lcddev.width-98)/2,65,(lcddev.width-98)/2+98,85); //x1,y1,x2,y2
	Gui_StrCenter(0,67,BRED,BLUE,"Õº–Œœ‘ æ≤‚ ‘",16,1);

	Display_ButtonUp((lcddev.width-98)/2,95,(lcddev.width-98)/2+98,115); //x1,y1,x2,y2
	Gui_StrCenter(0,97,BLACK,GRAY0,"¥ø…´ÃÓ≥‰≤‚ ‘",16,1);
	
	Display_ButtonUp((lcddev.width-98)/2,125,(lcddev.width-98)/2+98,145); //x1,y1,x2,y2
	Gui_StrCenter(0,127,BLUE,GRAY0,"÷–Œƒœ‘ æ≤‚ ‘",16,1);

	Display_ButtonUp((lcddev.width-98)/2,155,(lcddev.width-98)/2+98,175); //x1,y1,x2,y2
	Gui_StrCenter(16,157,RED,GRAY0,"Õº∆¨œ‘ æ≤‚ ‘",16,1);
	delay_ms(1500);
	delay_ms(500);
}

/*****************************************************************************
 * @name       :void main_test(void)
 * @date       :2018-08-09 
 * @function   :Drawing the main Interface of the Comprehensive Test Program
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void main_test(void)
{
	DrawTestPage("≤‚ ‘≤‚ ‘");	
	LCD_ShowString(30, 30, 12,"20:22", 1);
//	Gui_StrCenter(0,78,RED,BLUE,"»´∂ØµÁ◊”",16,1);//æ”÷–œ‘ æ
//	Gui_StrCenter(0,95,RED,BLUE,"◊€∫œ≤‚ ‘≥Ã–Ú",16,1);//æ”÷–œ‘ æ	
//	Gui_StrCenter(0,112,GREEN,BLUE,"1.28\" GC9A01",16,1);//æ”÷–œ‘ æ
//	Gui_StrCenter(0,129,GREEN,BLUE,"240X240",16,1);//æ”÷–œ‘ æ
//	Gui_StrCenter(0,146,BLUE,BLUE,"2022-09-16",16,1);//æ”÷–œ‘ æ
}

/*****************************************************************************
 * @name       :void Test_Color(void)
 * @date       :2018-08-09 
 * @function   :Color fill test(white,black,red,green,blue)
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Test_Color(void)
{
	//DrawTestPage("≤‚ ‘1:¥ø…´ÃÓ≥‰≤‚ ‘");
	LCD_Fill(0,0,lcddev.width-1,lcddev.height-1,WHITE);
	Show_Str(40,30,BLACK,YELLOW,"WHITE",16,1);delay_ms(1000);
	LCD_Fill(0,0,lcddev.width-1,lcddev.height-1,BLACK);
	Show_Str(40,30,WHITE,YELLOW,"BLACK",16,1);delay_ms(1000);
	LCD_Fill(0,0,lcddev.width-1,lcddev.height-1,RED);
	Show_Str(40,30,BLUE,YELLOW,"RED",16,1);delay_ms(800);
	LCD_Fill(0,0,lcddev.width-1,lcddev.height-1,GREEN);
	Show_Str(40,30,BLUE,YELLOW,"GREEN",16,1);delay_ms(800);
	LCD_Fill(0,0,lcddev.width-1,lcddev.height-1,BLUE);
	Show_Str(40,30,RED,YELLOW,"BLUE",16,1);delay_ms(800);
}

/*****************************************************************************
 * @name       :void Test_FillRec(void)
 * @date       :2018-08-09 
 * @function   :Rectangular display and fill test
								Display red,green,blue,yellow,pink rectangular boxes in turn,
								1500 milliseconds later,
								Fill the rectangle in red,green,blue,yellow and pink in turn
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Test_FillRec(void)
{
	u8 i=0;
	DrawTestPage("GUIæÿ–ŒÃÓ≥‰≤‚ ‘");
	LCD_Fill(0,30,lcddev.width-1,lcddev.height-32,WHITE);
	for (i=0; i<5; i++) 
	{
		POINT_COLOR=ColorTab[i];
		LCD_DrawRectangle(lcddev.width/2-40+(i*16),lcddev.height/2-40+(i*13),lcddev.width/2-40+(i*16)+30,lcddev.height/2-40+(i*13)+30); 
	}
	delay_ms(1500);	
	LCD_Fill(0,30,lcddev.width-1,lcddev.height-32,WHITE);
	for (i=0; i<5; i++) 
	{
		POINT_COLOR=ColorTab[i];
		LCD_DrawFillRectangle(lcddev.width/2-40+(i*16),lcddev.height/2-40+(i*13),lcddev.width/2-40+(i*16)+30,lcddev.height/2-40+(i*13)+30); 
	}
	delay_ms(1500);
}

/*****************************************************************************
 * @name       :void Test_Circle(void)
 * @date       :2018-08-09 
 * @function   :circular display and fill test
								Display red,green,blue,yellow,pink circular boxes in turn,
								1500 milliseconds later,
								Fill the circular in red,green,blue,yellow and pink in turn
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Test_Circle(void)
{
	u8 i=0;
	DrawTestPage("GUIª≠‘≤ÃÓ≥‰≤‚ ‘");
	LCD_Fill(0,30,lcddev.width-1,lcddev.height-32,WHITE);
	for (i=0; i<5; i++)  
		gui_circle(lcddev.width/2-40+(i*15),lcddev.height/2-25+(i*13),ColorTab[i],15,0);
	delay_ms(1500);	
	LCD_Fill(0,30,lcddev.width-1,lcddev.height-32,WHITE);
	for (i=0; i<5; i++) 
	  	gui_circle(lcddev.width/2-40+(i*15),lcddev.height/2-25+(i*13),ColorTab[i],15,1);
	delay_ms(1500);
}

/*****************************************************************************
 * @name       :void English_Font_test(void)
 * @date       :2018-08-09 
 * @function   :English display test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void English_Font_test(void)
{
	DrawTestPage("”¢Œƒœ‘ æ≤‚ ‘");
	Show_Str(50,77,BLUE,YELLOW,"6X12:abcdefgh01234567",12,0);
	Show_Str(50,89,BLUE,YELLOW,"6X12:ABCDEFGH01234567",12,1);
	Show_Str(50,101,BLUE,YELLOW,"6X12:~!@#$%^&*()_+{}:",12,0);
	Show_Str(50,114,BLUE,YELLOW,"8X16:abcde01234",16,0);
	Show_Str(50,130,BLUE,YELLOW,"8X16:ABCDE01234",16,1);
	Show_Str(50,146,BLUE,YELLOW,"8X16:~!@#$%^&*()",16,0); 
	delay_ms(1200);
}

/*****************************************************************************
 * @name       :void Test_Triangle(void)
 * @date       :2018-08-09 
 * @function   :triangle display and fill test
								Display red,green,blue,yellow,pink triangle boxes in turn,
								1500 milliseconds later,
								Fill the triangle in red,green,blue,yellow and pink in turn
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Test_Triangle(void)
{
	u8 i=0;
	DrawTestPage("GUI TriÃÓ≥‰≤‚ ‘");
	LCD_Fill(0,30,lcddev.width-1,lcddev.height-32,WHITE);
	for(i=0;i<5;i++)
	{
		POINT_COLOR=ColorTab[i];
		Draw_Triangel(lcddev.width/2-40+(i*15),lcddev.height/2-12+(i*11),lcddev.width/2-25-1+(i*15),lcddev.height/2-12-26-1+(i*11),lcddev.width/2-10-1+(i*15),lcddev.height/2-12+(i*11));
	}
	delay_ms(1500);	
	LCD_Fill(0,30,lcddev.width-1,lcddev.height-32,WHITE);
	for(i=0;i<5;i++)
	{
		POINT_COLOR=ColorTab[i];
		Fill_Triangel(lcddev.width/2-40+(i*15),lcddev.height/2-12+(i*11),lcddev.width/2-25-1+(i*15),lcddev.height/2-12-26-1+(i*11),lcddev.width/2-10-1+(i*15),lcddev.height/2-12+(i*11));
	}
	delay_ms(1500);
}

/*****************************************************************************
 * @name       :void Chinese_Font_test(void)
 * @date       :2018-08-09 
 * @function   :chinese display test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Chinese_Font_test(void)
{	
	DrawTestPage("÷–Œƒœ‘ æ≤‚ ‘");
	Show_Str(45,82,BLUE,YELLOW,"16X16:»´∂ØµÁ◊”ª∂”≠",16,0);
	Show_Str(45,102,BLUE,YELLOW,"24X24:÷–Œƒ≤‚ ‘",24,1);
	Show_Str(45,127,BLUE,YELLOW,"32X32:◊÷ÃÂ≤‚ ‘",32,1);
	delay_ms(1200);
}

/*****************************************************************************
 * @name       :void Pic_test(void)
 * @date       :2018-08-09 
 * @function   :picture display test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Pic_test(void)
{
	DrawTestPage("Õº∆¨œ‘ æ≤‚ ‘");
	//LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	Gui_Drawbmp16(50,90,40,40,gImage_qq);
	Show_Str(50+12,135,BLUE,YELLOW,"QQ",16,1);
	Gui_Drawbmp16(100,90,40,40,gImage_qq);
	Show_Str(100+12,135,BLUE,YELLOW,"QQ",16,1);
	Gui_Drawbmp16(150,90,40,40,gImage_qq);
	Show_Str(150+12,135,BLUE,YELLOW,"QQ",16,1);
	delay_ms(1200);
}

/*****************************************************************************
 * @name       :void Rotate_Test(void)
 * @date       :2018-08-09 
 * @function   :rotate test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Rotate_Test(void)
{
	u8 i=0;
	u8 *Direction[4]={"Rotation:0","Rotation:90","Rotation:180","Rotation:270"};
	
	for(i=0;i<4;i++)
	{
	LCD_direction(i);
	DrawTestPage("∆¡ƒª–˝◊™≤‚ ‘");
	Show_Str(40,35,BLUE,YELLOW,Direction[i],16,1);
	Gui_Drawbmp16(40,55,40,40,gImage_qq);
	delay_ms(1000);delay_ms(1000);
	}
	LCD_direction(USE_HORIZONTAL);
}


