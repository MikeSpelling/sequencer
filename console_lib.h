/*
 * Some simple functions for use in the console window
 * These are functions which are supported by Borland compilers,
 * but not Microsoft.
 *
 * Peter Mendham : 09/01/2006
 * (c) University of York
 */

#ifndef __CONSOLE_LIB_H__
#define __CONSOLE_LIB_H__

// Clear the console window
void clrscr();

// Set the console cursor position
void gotoxy(int x,  int y);

#endif // __CONSOLE_LIB_H__