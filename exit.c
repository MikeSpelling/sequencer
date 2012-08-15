#include "console_lib.h"
#include "header.h"

void exit_program(void)
{
	//	Clear the screen and call the freelist function for each sequence to ensure all memory is freed
	clrscr();
	
	freelist(&first_q);
	freelist(&first_w);
	freelist(&first_e);
	freelist(&first_r);
	freelist(&first_t);
	freelist(&first_y);	

	return;
}