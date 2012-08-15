#include "header.h"
#include <stdlib.h>

void before(int key)
{
	//	Create a pointer to structure note_info called last
	note_info *last;

	//	Set the pointer last to the current structure
	last = current;

	//	Depending on which sequence is being used, go the first structure in the appropriate sequence
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

	/*
	 *	While the pointer next in the current structure doesnt equal the pointer last, cycle
	 *	through the list by setting previous to current, and then current to the structure pointed
	 *	at by next
	 */

	while((current->next != last) && (current != last))
	{
		previous = current;
		current = current->next;
	}

	//	If at the first note in the sequence reset the pointer previous to Null
	if(previous == current)
		previous = NULL;
	
	return;
}