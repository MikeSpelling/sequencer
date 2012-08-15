#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include "console_lib.h"
#include "header.h"

void remove_note(int key)
{
	//Declare the local variable choice
	char choice;


	if(current->next == NULL)
	{
		if(previous == NULL)
		{

			/*
			 *	If the current note is the last in the sequence then warn the user, prompt whether to remove
			 *	the whole sequence and if so call the freelist function for the appropriate sequence to remove it from
			 *	memory.
			 */

			clrscr();
			gotoxy(3, 3);
			printf("Last note in sequence");
			gotoxy(3, 5);
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
				getch();
			}
		}
		else
		{
			/*
			 *	If at the end note in the sequence call the before function to go to the previous note, free the
			 *	structure pointed at by the pointer next in the current structure then set this pointer to Null
			 */

			before(key);
			free(current->next);
			current->next = NULL;
		}
	}
	else 
	{
		if(previous == NULL)
		{
			/*
			 *	If at the first note in the sequence set previous to current, current to the next note then free the memory
			 *	of the previous structure. Reset previous to Null and depending on which sequence is being edited assign
			 *	the appropriate first pointer to the current structure.
			 */

			previous = current;
			current = current->next;
			free(previous);
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
		else
		{

			/*
			 *	If in the middle of a sequence set the pointer next in the previous structure to the pointer next of the
			 *	current structure. Free the current structure from memory then set current to the structure pointed at 
			 *	by the pointer next in the previous structure
			 */

			previous->next = current->next;
			free(current);
			current = previous->next;
		}
	}

	return;
}