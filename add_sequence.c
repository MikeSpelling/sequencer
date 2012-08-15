#include <stdio.h>
#include "console_lib.h"
#include "midi_lib.h"
#include "header.h"

void add_sequence()
{

	//	Declare and initiate local variables
	int count;
	char key[2];
	int instrument;
	int number;
	int channel;

	count = 0;
	instrument = -1;
	number = -1;

	//	Clear the screen
	clrscr();
	
	//	While user input is not in expected range prompt user for key
	do
	{
		fflush(stdin);
		printf("Assign sequence to which key? [Q W E R T Y]: ");
		scanf("%s", &key[0]);

		if((key[0] != 'q' && key[0] != 'w' && key[0] != 'e' && key[0] != 'r' && key[0] != 't' && key[0] != 'y') || key[1] != '\0')
			printf("Invalid character\n\n");
	}
	while((key[0] != 'q' && key[0] != 'w' && key[0] != 'e' && key[0] != 'r' && key[0] != 't' && key[0] != 'y') || key[1] != '\0');

	//	While user input is not in expected range prompt user for instrument
	while(instrument < 0 || instrument > 127)
	{
		printf("Choose instrument [0 - 127]: ");
		scanf("%d", &instrument);
		if(instrument < 0 || instrument > 127)
			printf("Invalid character\n\n");
	}
	
	//	While user input is not in expected range prompt user for number of notes in sequence
	while(number < 0)
	{
		printf("Number of notes in sequence: ");
		scanf("%d", &number);
		if(number < 0)
			printf("Invalid character\n\n");
	}

	//	Create a structure of type note_info and assign the pointer current to it
	current = (note_info*)malloc(sizeof(note_info));

	/*	
	 *	Depending on which key the sequence is to be assigned to remove any seqeunce 
	 *	previously associated with the key, set the appropriate channel and assign the 
	 *	appropriate first pointer to the current structure
	 */

	switch(key[0])
	{
	case 'q':
		{
			freelist(&first_q);
			channel = 1;			
			first_q = current;			
			break;
		}
	case 'w':
		{
			freelist(&first_w);
			channel = 2;
			first_w = current;			
			break;
		}
	case 'e':
		{
			freelist(&first_e);
			channel = 3;
			first_e = current;			
			break;
		}
	case 'r':
		{
			freelist(&first_r);
			channel = 4;
			first_r = current;			
			break;
		}
	case 't':
		{
			freelist(&first_t);
			channel = 5;
			first_t = current;			
			break;
		}
	case 'y':
		{
			freelist(&first_y);
			channel = 6;
			first_y = current;			
			break;
		}
	}

	//	Assign the chosen instrument to the channel
	program_change(channel, instrument);			

	//	Clear the screen, prompt the user for note information and initiate variables
	clrscr();

	printf("Enter pitch, velocity and duration of notes. Enter 0 velocity for rests:\n");

	current->pitch = -1;
	current->velocity = -1;
	current->duration = -1;

	//	Create a loop which cycles while count is lower than the number of notes in sequence chosen by the user
	while(count < number)
	{
		//	Increment count each cycle
		count++;

		//	While user input is not in expected range prompt user for Pitch
		while((current->pitch < 0) || (current->pitch > 127))
		{	
			printf("\nPitch: ");
			scanf("%d", &current->pitch);
			if((current->pitch < 0) || (current->pitch > 127))
				printf("Invalid character\n");
		}

		//	While user input is not in expected range prompt user for Velocity
		while((current->velocity < 0) || (current->velocity > 127))
		{
			printf("Velocity: ");
			scanf("%d", &current->velocity);
			if((current->velocity < 0) || (current->velocity > 127))
				printf("Invalid character\n\n");
		}

		//	While user input is not in expected range prompt user for Duration
		while(current->duration < 1)
		{
			printf("Duration: ");		
			scanf("%d", &current->duration);
			if(current->duration < 1)
				printf("Invalid character\n\n");
		}

		//	Assign the chosen channel to the channel in the current structure
		current->channel = channel;			

		/*
		 *	If not at the end of the sequence set the previous pointer to the current structure,
		 *	create a new structure the assign the pointer next in previous to the new structure
		 */

		if(count < number)
		{
		previous = current;
		current = (note_info*)malloc(sizeof(note_info));
		previous->next = current;
		}

		//	If at the end of the sequence set the next pointer to Null
		else
			current->next = NULL;
	}
	return;
}