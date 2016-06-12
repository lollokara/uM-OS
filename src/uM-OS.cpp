/******************************************************************************
* uM.cpp
* CiAo CoMe StAi
* Original Creation Date: 10/06/2016
*
* Distributed as-is; no warranty is given.
******************************************************************************/

#include <iostream>
#include "oled/Edison_OLED.h"
#include "gpio/gpio.h"
#include "math.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#define BoxSizex		64
#define BoxSizey		11
#define MAXPOS			4
#define MAXPAGE			1
/*
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>

#include <unistd.h>

#include <array>

#include <cstdio>
#include <memory>
*/



using namespace std;

void setupOLED();
void cleanUp();
void drawBatt(bool BoxSelected);
void Menu();
void DrawMenu(int CursorPos);
void DrawPage(int Page, int CursorPos);
void Selected(int Page,int CursorPos);
void Info();
void Voltmeter();
float ReadADC(float PreviousValue);
void displayBar(float Value,int Range);



edOLED oled;
gpio BUTTON_UP(47, INPUT);
gpio BUTTON_DOWN(44, INPUT);
gpio BUTTON_LEFT(165, INPUT);
gpio BUTTON_RIGHT(45, INPUT);
gpio BUTTON_SELECT(48, INPUT);
gpio BUTTON_A(49, INPUT);
gpio BUTTON_B(46, INPUT);

int main(int argc, char * argv[])
{
	setupOLED();
	Menu();

}

void setupOLED()
{
	oled.begin();
	oled.clear(PAGE);
	oled.display();
	oled.setFontType(1);
	oled.setCursor(10,15);
	oled.print("uM-OS");
	oled.setFontType(0);
	for(int i=0;i<101;i=i+5){
		oled.setCursor(24,36);
		oled.print(i);
		oled.setCursor(42,36);
		oled.print("%");
		oled.rect(0,30,64,4);
		float size=i/(100.00/64.00);
		oled.rectFill(0,31,int(size),2);
		oled.display();
	}
	oled.clear(PAGE);
}

void drawBatt(bool BoxSelected)
{
	FILE *fp;
	char Batt[32];
	fp = popen("battery-voltage", "r");
	fgets(Batt, 32, fp);
	fgets(Batt, 32, fp);
	pclose(fp);
	char *p = Batt;
	int val=0;
	while (*p) { // While there are more characters to process...
	    if (isdigit(*p)) { // Upon finding a digit, ...
	        val = strtol(p, &p, 10); // Read a number, ...
	        printf("%d\n", val); // and print it.
	        break;
	    } else { // Otherwise, move on to the next character.
	        p++;
	    }
	}
	if(BoxSelected) oled.setColor(BLACK);
	else oled.setColor(WHITE);
	oled.rect(51,0,12,4);
	oled.rectFill(52,1,(val+1)/10,2);
	oled.rect(63,1,1,2);
	oled.setColor(WHITE);
}

void Menu(){
	int CursorPos=0;
	int Page=0;
	while(0==0){
		DrawMenu(CursorPos);
		drawBatt(false);
		DrawPage(Page,CursorPos);
		oled.display();
		while ((BUTTON_UP.pinRead() == HIGH) && (BUTTON_DOWN.pinRead() == HIGH) && (BUTTON_SELECT.pinRead() == HIGH) && (BUTTON_LEFT.pinRead() == HIGH) && (BUTTON_RIGHT.pinRead() == HIGH)) usleep(100000);
		if (BUTTON_UP.pinRead() == LOW){
			if(CursorPos==0){
				CursorPos=MAXPOS-1;
				if(Page==0){
					Page=MAXPAGE;
				}
				else Page--;
			}
			else CursorPos--;
		}
		if (BUTTON_DOWN.pinRead() == LOW){
					if(CursorPos==MAXPOS-1){
						CursorPos=0;
						if(Page==MAXPAGE){
							Page=0;
						}
						else Page++;
					}
					else CursorPos++;
				}
		if (BUTTON_RIGHT.pinRead() == LOW){
			if(Page==MAXPAGE){
				Page=0;
			}
			else Page++;
		}

		if (BUTTON_LEFT.pinRead() == LOW){

			if(Page==0){
				Page=MAXPAGE;
			}
			else Page--;

		}
		if (BUTTON_SELECT.pinRead() == LOW){
			Selected(Page,CursorPos);
		}
	}
}

void DrawMenu(int CursorPos){
	for(int i=0; i<MAXPOS;i++){
		oled.rect(0,BoxSizey*i+5,BoxSizex,BoxSizey);
		if(CursorPos==i) oled.setColor(WHITE);
		else oled.setColor(BLACK);
		oled.rectFill(0,BoxSizey*i+5,BoxSizex,BoxSizey-1);
		oled.setColor(WHITE);
	}
}

void DrawPage(int Page, int CursorPos){
	const char* Voices[MAXPAGE+1][MAXPOS];
	Voices[0][0]="V";
	Voices[0][1]="OMH";
	Voices[0][2]="mA";
	Voices[0][3]="A";
	Voices[1][0]="ADC  Sett.";
	Voices[1][1]="Gen. Sett.";
	Voices[1][2]="Wifi Sett.";
	Voices[1][3]="Info";
	for(int i=0;i<MAXPOS;i++){
		if(CursorPos==i) oled.setColor(BLACK);
		else oled.setColor(WHITE);
		oled.setCursor(1,6+BoxSizey*i);
		printf("%d  %d",Page,i);
		oled.print(Voices[Page][i]);
		oled.setColor(WHITE);
	}

}


void Selected(int Page,int CursorPos){
	switch (Page) {
	case 0:
		switch (CursorPos) {
		case 0:
			Voltmeter();
			break;
		}
		break;
	case 1:
		switch (CursorPos) {
		case 0:
			cleanUp();
			break;
		case 3:
			Info();

		break;

		}
		break;
	}
}


void Info(){
	cleanUp();
	oled.setFontType(1);
	oled.setCursor(10,10);
	oled.print("uM-OS");
	oled.setFontType(0);
	oled.setCursor(12,25);
	oled.print("V 0.2A");
	oled.setCursor(52,40);
		oled.print("LK");
	oled.display();
	while ((BUTTON_SELECT.pinRead() == HIGH));
	if (BUTTON_SELECT.pinRead() == LOW){
		Menu();
	}

}

void cleanUp()
{
	oled.clear(PAGE);
	oled.display();
}


void Voltmeter(){
	oled.clear(PAGE);
	float InitValue=ReadADC(0);
	int Range=0;
	usleep((500)*1000);
	while (true){
		float Value=0;
		if(InitValue<1) Range=1;
		else if(InitValue<10) Range=10;
		else if (InitValue<20) Range=20;
		switch(Range){
		case 1:
			Value=(int(ReadADC(InitValue)*1000))/1000.0000;
			break;
		case 10:
			Value=(int(ReadADC(InitValue)*100))/100.000;
			break;
		case 20:
			Value=(int(ReadADC(InitValue)*100))/100.00;
			break;
		}
		displayBar(Value,Range);
		oled.setFontType(1);
		oled.setCursor(55,15);
		oled.print("V");
		oled.setCursor(5,15);
		oled.print(Value);
		oled.setFontType(0);

		oled.display();
		if(BUTTON_SELECT.pinRead() == LOW) break;
	}
}


void displayBar(float Value,int Range){
	oled.rect(0,34,64,4);
	float size=Value/(Range/64.000);
	oled.rectFill(0,35,int(size),2);
	oled.setFontType(0);
	oled.setCursor(50,40);
	oled.print(Range);

}


float ReadADC(float PreviousValue){
	float Val=0;
	if(PreviousValue==0) Val=(random()/(RAND_MAX/20.0000));
	printf("Val= %f  ",Val);
	if(PreviousValue==0) return(Val);
	if(PreviousValue!=0) Val=((1-(random()/(RAND_MAX/2.0000)))/(1+(2.0001-PreviousValue/10.00)));///((101.0000-PreviousValue)*100.000
	return(PreviousValue+Val);
}




/*
void startScreen()
{
	oled.clear(PAGE);
	oled.setCursor(14, 5);
	oled.print("Press A");
	oled.setCursor(2, 13);
	oled.print("for single");
	oled.setCursor(14, 30);
	oled.print("Press B");
	oled.setCursor(6, 38);
	oled.print("for multi");
	// Call display to actually draw it on the OLED:
	oled.display();

	// Wait for either button A or B to be pressed:
	while ((BUTTON_A.pinRead() == HIGH) && (BUTTON_B.pinRead() == HIGH))
		;
	// If button A is pressed, play single player
	if (BUTTON_A.pinRead() == LOW)
		playMode = SINGLE_PLAYER;
	// If button B is pressed, play mutli-player
	else if (BUTTON_B.pinRead() == LOW)
		playMode = MULTI_PLAYER;
}

// Update the positions of the paddles:
void updatePaddlePositions()
{
	// Update player 1's paddle:
	if (BUTTON_UP.pinRead() == LOW)
	{
		player1PosY--;
	}
	if (BUTTON_DOWN.pinRead() == LOW)
	{
		player1PosY++;
	}
	player1PosY = constrainPosition(player1PosY);

	// Move player 2 paddle:
	if (playMode == SINGLE_PLAYER)
	{
		// Update AI's paddle:
		// Follow along with the ball's position:
		if (player2PosY < ballPosY)
		{
			player2PosY += enemyVelY;
		}
		else if(player2PosY > ballPosY)
		{
			player2PosY -= enemyVelY;
		}
	}
	else if (playMode == MULTI_PLAYER)
	{
		if (BUTTON_A.pinRead() == LOW)
		{
			player2PosY--;
		}
		if (BUTTON_B.pinRead() == LOW)
		{
			player2PosY++;
		}
	}
	player2PosY = constrainPosition(player2PosY);
}

// Constrain a paddle's position to within the display's border
float constrainPosition(float position)
{
	float newPaddlePosY = position;

	if (position - halfPaddleHeight < 0)
	{
		newPaddlePosY = halfPaddleHeight;
	}
	else if (position + halfPaddleHeight > LCDHEIGHT)
	{
		newPaddlePosY = LCDHEIGHT - halfPaddleHeight;
	}

	return newPaddlePosY;
}

// Move the ball and re-calculate its position:
void moveBall()
{
	ballPosY += ballVelY;
	ballPosX += ballVelX;

	// Top and bottom wall collisions
	if (ballPosY < ballRadius)
	{
		ballPosY = ballRadius;
		ballVelY *= -1.0;
	}
	else if (ballPosY > LCDHEIGHT - ballRadius)
	{
		ballPosY = LCDHEIGHT - ballRadius;
		ballVelY *= -1.0;
	}

	// Left and right wall collisions
	if (ballPosX < ballRadius)
	{
		ballPosX = ballRadius;
		ballVelX = ballSpeedX;
		player2Score++;
	}
	else if (ballPosX > LCDWIDTH - ballRadius)
	{
		ballPosX = LCDWIDTH - ballRadius;
		ballVelX *= -1.0 * ballSpeedX;
		playerScore++;
	}

	// Paddle collisions
	if (ballPosX < player1PosX + ballRadius + halfPaddleWidth)
	{
		if (ballPosY > player1PosY - halfPaddleHeight - ballRadius &&
				ballPosY < player1PosY + halfPaddleHeight + ballRadius)
		{
			ballVelX = ballSpeedX;
			ballVelY = 2.0 * (ballPosY - player1PosY) / halfPaddleHeight;
		}
	}
	else if (ballPosX > player2PosX - ballRadius - halfPaddleWidth)
	{
		if (ballPosY > player2PosY - halfPaddleHeight - ballRadius &&
				ballPosY < player2PosY + halfPaddleHeight + ballRadius)
		{
			ballVelX = -1.0 * ballSpeedX;
			ballVelY = 2.0 * (ballPosY - player2PosY) / halfPaddleHeight;
		}
	}
}

// Draw the paddles, ball and score:
void drawGame()
{
	oled.clear(PAGE);

	drawScore(playerScore, player2Score);
	drawPaddle(player1PosX, player1PosY);
	drawPaddle(player2PosX, player2PosY);
	drawBall(ballPosX, ballPosY);

	oled.display();
}

// Draw the two score integers on the screen
void drawScore(int player1, int player2)
{
	oled.setCursor(10, 2);
	oled.print(player1);
	oled.setCursor(50, 2);
	oled.print(player2);
}

// Draw a paddle, given it's x and y coord's
void drawPaddle(int x, int y)
{
	oled.rect(x - halfPaddleWidth,
			y - halfPaddleHeight,
			paddleWidth,
			paddleHeight);
}

// Draw a ball, give it's x and y coords
void drawBall(int x, int y)
{
	oled.circle(x, y, 2);
}

// Check if either player has won.
// Returns:
//	0 - Neither player has won.
//  1 - Player 1 has won
//  2 - Player 2 has won
int checkWin()
{
	if (playerScore >= scoreToWin)
	{
		return PLAYER_1_WIN;
	}
	else if (player2Score >= scoreToWin)
	{
		return PLAYER_2_WIN;
	}

	return 0;
}

// Draw the win screen.
// Keep it up for 5 seconds.
// Then go back to the splash screen.
void drawWin(int player)
{
	oled.setCursor(10, 2);
	oled.clear(PAGE);
	if (player == PLAYER_1_WIN)
	{
		oled.print("Player 1");
	}
	else if (player == PLAYER_2_WIN)
	{
		oled.print("Player 2");
	}
	oled.setCursor(20, 12);
	oled.print("Wins!");
	oled.display();

	usleep(5000000);
}
*/

