#include <stdio.h>
#include "console_lib.h"

void user_interface(void)
{
	//	Clear the screen and empty the standard input
	clrscr();
	fflush(stdin);

	//	Display options to the user to create an interface
	gotoxy(30, 3);
	printf("[ MIDI Sequencer ]");

	gotoxy(3, 7);
	printf("[a] Add a Sequence");

	gotoxy(3, 9);
	printf("[e] Edit a Sequence");

	gotoxy(3, 11);
	printf("[p] Enter play mode");

	gotoxy(3, 23);
	printf("[Backspace] Quit");

	return;
}