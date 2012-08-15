#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "console_lib.h"
#include "header.h"

void insert_note(int key)
{
	//	Declare the local variable choice
	int choice;

	//	Clear the screen and prompt the user whether to insert the note before or after the current note
	clrscr();

	gotoxy(3, 3);
	printf("Insert note before or after current note? [b/a]");
	choice = getch();
	clrscr();

	switch(choice)
	{

	//	If the user pressed backspace return to the main menu
	case BACK:
		{
			return;
		}
	 
	//	If the user pressed b then insert a note before the current one
	case 'b':
		{
			/*
			 *	Check whether the current note is at the beginning of the sequence. If not then create a 
			 *	new structure and assign current to it. Set the pointer next in the current
			 *	structure to point to the address of the pointer next in the previous structure 
			 *	then set the pointer next in the previous structure to the current, new structure.
			 */
			if(previous != NULL)
			{				
				current = (note_info*)malloc(sizeof(note_info));
				current->next = previous->next;
				previous->next = current;				
			}

			/*
			 *	If the current note is at the beginning of the sequence set previous to the current structure, create
			 *	a new structure and assign it to the pointer current, then link it up by setting the pointer next in the
			 *	current structure to the previous one. Set previous to Null to show that it is at the beginning of the list
			 *	then, depending on which sequence is being edited set the appropriate first pointer to the current structure.
			 */

			else
			{
				previous = current;
				current = (note_info*)malloc(sizeof(note_info));
				current->next = previous;
				previous = NULL;

				switch(key)
				{
				case 'q':
					{			
						first_q = current;
						break;
					}
				case 'w':
					{
						first_w = current;
						break;
					}
				case 'e':
					{
						first_e = current;
						break;
					}
				case 'r':
					{
						first_r = current;
						break;
					}
				case 't':
					{
						first_t = current;
						break;
					}
				case 'y':
					{
						first_y = current;
						break;
					}
				}
			}
			break;
		}

	//	If the user pressed a then insert a new note after the current one
	case 'a':
		{
			/*
			 *	Check whether the current note is at the end of the sequence. If not then set previous to the current
			 *	structure then create a new structure and assign current to it. Then set the pointer next in the current
			 *	structure to the pointer next of the previous structure, then set the pointer next in the previous
			 *	structure to the current one.
			 */

			if(current->next != NULL)
			{
				previous = current;
				current = (note_info*)malloc(sizeof(note_info));
				current->next = previous->next;	
				previous->next = current;							
			}

			/*
			 *	If the current note is at the end of the sequence set previous to the current structure, assign current
			 *	to a new structure then set the pointer next in the previous structure to the current one, and to
			 *	show the end of the sequence set the pointer next to Null.
			 */

			else
			{
				previous = current;
				current = (note_info*)malloc(sizeof(note_info));
				previous->next = current;
				current->next = NULL;
			}
			break;
		}
	}

	//	Once the new note has been created prompt the user for the details of the note and save them in the current structure

	current->pitch = -1;
	current->velocity = -1;
	current->duration = -1;

	while((current->pitch < 0) || (current->pitch > 127))
	{	
		printf("\Pitch: ");
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

	//	Depending on which sequence is being edited set the channel to the appropriate value
	switch(key)
	{
	case 'q':
		{
			current->channel = 1;
			break;
		}
	case 'w':
		{
			current->channel = 2;
			break;
		}
	case 'e':
		{
			current->channel = 3;
			break;
		}
	case 'r':
		{
			current->channel = 4;
			break;
		}
	case 't':
		{
			current->channel = 5;
			break;
		}
	case 'y':
		{
			current->channel = 6;
			break;
		}
	}
	return;
}