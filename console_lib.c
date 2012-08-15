/*
 * Some simple functions for use in the console window
 * These are functions which are supported by Borland compilers,
 * but not Microsoft.
 *
 * Peter Mendham : 09/01/2006
 * (c) University of York
 */

#include <windows.h>

/*
 * Clear the console screen
 */
void clrscr()
{
	system("cls");
}

/*
 * Set the console cursor location
 */
void gotoxy(int x, int y)
{
	COORD coord;

	coord.X = x - 1;
	coord.Y = y - 1;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
