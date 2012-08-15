#include <stdio.h>
#include <conio.h>
#include "header.h"

int main(void)
{
	//	Create a variable integer loop and set it to 1
	int loop;

	loop = 1;

	//	Use the variable loop to create a while loop
	while(loop == 1)
	{
		//	Call the user_interface function to display the user interface
		user_interface();

		//	Get input from the user and depending on which key was pressed all the appropriate function
		switch(getch())
		{

		//	If 'a' was pressed call the add_sequence function to create a new sequence of notes
		case 'a':
			{
				add_sequence();
				break;
			}

		//	If 'e' was pressed call the edit_sequence function to change a stored sequence
		case 'e':
			{
				edit_sequence();
				break;
			}

		//	If 'p' was pressed call the play function to playback a sequence
		case 'p':
			{
				play();
				break;
			}

		//	If backspace was hit call the exit_program function to clear memory then set loop to 0 to finish the while loop
		case BACK:
			{
				exit_program();
				loop = 0;
				break;
			}

		//	In any other case just clear the input and repeat the loop
		default:
			{
				fflush(stdin);
				break;
			}
		}
	}

	return 0;
}