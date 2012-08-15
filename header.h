//	Define numbers to replace certain words to make the code easier to read

#define OFF 0
#define BACK 8
#define SPACE 32
#define TAB 9

//	Define a structure called note_info containing 4 integers and a pointer to the same structure called next
typedef struct long_note
{
	int pitch;
	int channel;
	int velocity;
	int duration;
	struct long_note *next;
}note_info;

//	Define pointers to the structure note_info
note_info *first_q;
note_info *first_w;
note_info *first_e;
note_info *first_r;
note_info *first_t;
note_info *first_y;
note_info *current;
note_info *previous;

//	Declare all functions
void user_interface(void);
void add_sequence(void);
void edit_sequence(void);
void play(void);
void insert_note(int key);
void freelist(note_info **start);
void before(int key);
void remove_note(int key);
void exit_program(void);


