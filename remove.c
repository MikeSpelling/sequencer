#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include "console_lib.h"
#include "header.h"

void remove_note(int key)
{
	char choice;

	if(current->next == NULL)
	{
		if(previous == NULL)
		{
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
			before(key);
			free(current->next);
			current->next = NULL;
		}
	}
	else 
	{
		if(previous == NULL)
		{
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
			previous->next = current->next;
			free(current);
			current = previous->next;
		}
	}

	return;
}