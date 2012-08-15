#include <stdlib.h>
#include "header.h"

void freelist(note_info **start)
{
	/*
	 *	Create 2 pointers to structure note_info. Assign cur to the pointer passed into the function which is
	 *	the first note in the chosen sequence.
	 */

	note_info *cur = (*start);
	note_info *prev;

	//	Enter the loop while the current structure is not equal to Null
	while(cur != NULL)
	{
		//Set the pointer prev to cur, cur to the next structure in the list and free the previous structure
		prev = cur;
		cur = cur->next;
		free(prev);
	}

	//	Stop the pointer start pointing at memory by assigning it to Null
	*start = NULL;

	return;
}