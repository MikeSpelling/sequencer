#include <stdio.h>
#include <conio.h>
#include "console_lib.h"
#include "midi_lib.h"
#include "header.h"

void play()
{

	//	Declare local variables and initiate loop to 1
	char key;
	int loop;
	int repeat;

	loop = 1;

	//	Clear the screen and prompt the user which sequence to play
	clrscr();
	gotoxy(3, 3);
	printf("Press key to play back the assigned sequence...");

	//	Display the letter associated with any sequence that has been created
	if(first_q != NULL)
	{
		gotoxy(20, 10);
		printf("[Q]");
	}
	if(first_w != NULL)
	{
		gotoxy(25, 10);
		printf("[W]");
	}
	if(first_e != NULL)
	{
		gotoxy(30, 10);
		printf("[E]");
	}
	if(first_r != NULL)
	{
		gotoxy(35, 10);
		printf("[R]");
	}
	if(first_t != NULL)
	{
		gotoxy(40, 10);
		printf("[T]");
	}
	if(first_y != NULL)
	{
		gotoxy(45, 10);
		printf("[Y]");
	}

	gotoxy(3, 23);
	printf("Press backspace to stop then return to main menu");

	//	Once the user interface is displayed enter a loop while the variable loop is equal to 1
	while(loop == 1)
	{
		//	Empty the standard input and set repeat to 0
		fflush(stdin);

		repeat = 0;
		
		//	Get input from the user
		key = getch();

		/*
		 *	Check which key is pressed by the user and if the associated sequence is not empty set current to
		 *	the appropriate first structure. Set repeat to 1, to enter the playback loop. If the sequence is
		 *	empty set repeat to 0, to re-prompt the user for input and skip the playback loop.
		 */

		switch(key)
		{
		case 'q':
			{
				if(first_q != NULL)
				{
					repeat = 1;
					current = first_q;
				}
				else
					repeat = 0;
				break;
			}
		case 'w':
			{
				if(first_w != NULL)
				{
					repeat = 1;
					current = first_w;
				}
				else
					repeat = 0;
				break;
			}
		case 'e':
			{
				if(first_e != NULL)
				{
					repeat = 1;
					current = first_e;
				}
				else
					repeat = 0;
				break;
			}
		case 'r':
			{
				if(first_r != NULL)
				{
					repeat = 1;
					current = first_r;
				}
				else
					repeat = 0;
				break;
			}
		case 't':
			{
				if(first_t != NULL)
				{
					repeat = 1;
					current = first_t;
				}
				else
					repeat = 0;
				break;
			}
		case 'y':
			{
				if(first_y != NULL)
				{
					repeat = 1;
					current = first_y;
				}
				else
					repeat = 0;
				break;
			}

		//	If the user pressed backspace return to the main menu
		case BACK:
			{
				return;
			}
		}
		
		//	Enter the playback loop only if repeat equals 1
		while(repeat == 1)
		{
			/*	Play a midi note at the pitch, channel and velocity stored in the current structure, pause for the
			 *	duration stored in the current structure, then turn the note off.
			 */

			midi_note(current->pitch, current->channel, current->velocity);
			pause(current->duration);
			midi_note(current->pitch, current->channel, OFF);

			//	Go to the next note in the sequence
			current = current->next;

			//	If at the end of the sequence check which sequence is being played and return to the first note
			if(current == NULL)
			{
				switch(key)
				{			
				case 'q':
					{
						current = first_q;
						break;
					}
				case 'w':
					{
						current = first_w;
						break;
					}
				case 'e':
					{
						current = first_e;
						break;
					}
				case 'r':
					{
						current = first_r;
						break;
					}
				case 't':
					{
						current = first_t;
						break;
					}
				case 'y':
					{
						current = first_y;
						break;
					}
				}
			}

			//	If the user has pressed a key check what has been pressed
			if(kbhit() == 1)
			{
				switch(getch())
				{

				//	If the user pressed backspace set repeat to 0 to break the loop
				case BACK:
					{
						repeat = 0;
						break;
					}

				/*
				 *	If the user pressed any of the sequence associated keys check whether the sequence exists and if so
				 *	set the current note to the first in the new sequence.
				 */

				case 'q':
					{
						if(first_q != NULL)
						{
							current = first_q;
							key = 'q';
						}
						break;
					}
				case 'w':
					{
						if(first_w != NULL)
						{
							current = first_w;
							key = 'w';
						}
						break;
					}
				case 'e':
					{
						if(first_e != NULL)
						{
							current = first_e;
							key = 'e';
						}
						break;
					}
				case 'r':
					{
						if(first_r != NULL)
						{
							current = first_r;
							key = 'r';
						}
					}
				case 't':
					{
						if(first_t != NULL)
						{
							current = first_t;
							key = 't';
						}
						break;
					}
				case 'y':
					{
						if(first_y != NULL)
						{
							current = first_y;
							key = 'y';
						}
						break;
					}
				}

				//	Empty the standard input to reset kbhit()
				fflush(stdin);
			}
		}
	}

	return;
}