#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "console_lib.h"
#include "header.h"

void edit_sequence()
{
	//	Declare and initiate local variables
	char key;
	char choice;
	int check;
	int loop;

	check = 1;
	
	//	Clear the screen and display all stored sequences
	clrscr();

	gotoxy(3, 3);
	printf("Choose sequence stored [Backspace to return]");

	if(first_q != NULL)
	{
		gotoxy(3, 5);
		printf("[Q]");
	}
	if(first_w != NULL)
	{
		gotoxy(3, 7);
		printf("[W]");
	}
	if(first_e != NULL)
	{
		gotoxy(3, 9);
		printf("[E]");
	}
	if(first_r != NULL)
	{
		gotoxy(3, 11);
		printf("[R]");
	}
	if(first_t != NULL)
	{
		gotoxy(3, 13);
		printf("[T]");
	}
	if(first_y != NULL)
	{
		gotoxy(3, 15);
		printf("[Y]");
	}

	//	Enter a loop while the variable check equals 1
	while(check == 1)
	{
		//	Get user input
		key = getch();

		/*
		 *	Depending on which key was pressed check whether the associated sequence exists, if so set current 
		 *	to the appropriate first structure and set check to 0 to exit the loop
		 */

		switch(key)
		{
		case 'q':
			{
				if(first_q != NULL)
				{
					current = first_q;					
					check = 0;
				}
				break;
			}
		case 'w':
			{
				if(first_w != NULL)
				{
					current = first_w;
					check = 0;
				}
				break;
			}
		case 'e':
			{
				if(first_e != NULL)
				{
					current = first_e;
					check = 0;
				}
				break;
			}
		case 'r':
			{
				if(first_r != NULL)
				{
					current = first_r;
					check = 0;
				}
				break;
			}
		case 't':
			{
				if(first_t != NULL)
				{
					current = first_t;
					check = 0;
				}
				break;
			}
		case 'y':
			{
				if(first_y != NULL)
				{
					current = first_y;
					check = 0;
				}
				break;
			}
		case BACK:
			return;
		}
	}

	//	Set previous to Null and clear the screen
	previous = NULL;

	clrscr();

	//	Enter a loop while the current structure is not equal to Null
	while(current != NULL)
	{
		//	Clear the screen and initiate loop to 1
		clrscr();
		loop = 1;

		//	Print the details of the current note and the options the user can do
		gotoxy(3, 3);
		printf("Pitch: %d", current->pitch);
		gotoxy(3, 4);
		printf("Velocity: %d", current->velocity);
		gotoxy(3, 5);
		printf("Duration: %d", current->duration);
		
		gotoxy(3, 7);
		printf("[d] Delete Sequence");
		gotoxy(3, 8);
		printf("[e] Edit note");
		gotoxy(3, 9);
		printf("[r] Remove note");
		gotoxy(3, 10);
		printf("[i] Insert note");
		gotoxy(3, 12);
		printf("[Spacebar]  Next note");
		gotoxy(3, 13);
		printf("[Tab]       Previous note");
		gotoxy(3, 14);
		printf("[Backspace] Quit");

		//	Get input from the user
		choice = getch();

		switch(choice)
		{

		/*
		 *	If the user pressed spacebar then go to the next note by setting the previous to the current structure,
		 *	and setting current to the next structure. If at the end of the sequence print sequence over and allow the
		 *	user the choice to go back, else current will be at Null terminating the loop
		 */

		case SPACE:
			{
				previous = current;				
				current = current->next;
				if(current == NULL)
				{
					clrscr();
					gotoxy(30, 3);
					printf("[Sequence Over]");
					choice = getch();
					if(choice == TAB)
					{
						if(previous != NULL)
							before(key);
					}
				}
				break;
			}

		//	If the user pressed backspace return to the main menu
		case BACK:
			{				
				return;
			}

		//	If the user pressed tab then go to the previous note by calling the before function
		case TAB:
			{
				if(previous != NULL)
				{
					before(key);
				}
				break;
			}

		//	If the user pressed e then clear the screen and prompt the user to enter new details for the note
		case 'e':
			{
				clrscr();
				printf("Change note to:\n");

				current->pitch = -1;
				current->velocity = -1;
				current->duration = -1;
				
				while((current->pitch < 0) || (current->pitch > 127))
				{	
					printf("\nPitch: ");
					scanf("%d", &current->pitch);
					if((current->pitch < 0) || (current->pitch > 127))
						printf("Invalid character\n");
				}

				while((current->velocity < 0) || (current->velocity > 127))
				{
					printf("Velocity: ");
					scanf("%d", &current->velocity);
					if((current->velocity < 0) || (current->velocity > 127))
						printf("Invalid character\n\n");
				}

				while(current->duration < 1)
				{
					printf("Duration: ");		
					scanf("%d", &current->duration);
					if(current->duration < 1)
						printf("Invalid character\n\n");
				}				
				break;
			}

		//	If the user pressed r then check whether to delete note or not and if so call the remove_note function
		case 'r':
			{
				clrscr();
				gotoxy(3, 3);
				printf("Remove note) [y/n]");
				if(getch() == 'y')
					remove_note(key);
				break;
			}

		/*
		 *	If the user pressed d check whether to delete sequence or not and if so call the freelist function for the appropriate
		 *	sequence. Set current to Null, tell the user the sequence has been deleted, set loop to 0 to break the loop then wait for
		 *	a keypress. If the user chose not to delete the sequence clear the screen and set loop to 0 to break the loop.
		 */
			
		case 'd':
			{
				while(loop == 1)
				{
					clrscr();
					gotoxy(3, 3);
					printf("Delete sequence? [y/n]");
					choice = getch();
					
					if(choice == 'y')
					{
						switch(key)
						{
						case 'q':
							{
								freelist(&first_q);
								break;
							}
						case 'w':
							{
								freelist(&first_w);
								break;
							}
						case 'e':
							{
								freelist(&first_e);
								break;
							}
						case 'r':
							{
								freelist(&first_r);
								break;
							}
						case 't':
							{
								freelist(&first_t);
								break;
							}
						case 'y':
							{
								freelist(&first_y);
								break;
							}							
						}
						clrscr();
						current = NULL;				
						gotoxy(3, 3);
						printf("Sequence deleted");
						loop = 0;
						getch();
					}
					if(choice == 'n')
					{
						clrscr();
						loop = 0;
					}
				}
				break;
			}

		//	If the user pressed i to insert a note call the insert_note function
		case 'i':
			{
				insert_note(key);
				break;
			}
		}
	}

	return;
}
