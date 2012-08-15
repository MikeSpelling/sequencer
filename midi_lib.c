/*************************************************************************/
/*                                                                       */
/*      MIDI C library                                                   */
/*      ==============                                                   */
/*                                                                       */
/*      For full information see comments in midi_lib.h                  */
/*                                                                       */
/*************************************************************************/

// STANDARD INCLUDES 

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mmsystem.h>
#include <conio.h>
#include <wincon.h>

// MIDI LIB INCLUDE

#include "midi_lib.h"

// STRUCTURE DEFINITIONS, TYPE DEFINITIONS AND VARIABLES

/*
 *
 * UNION : 
 *
 * static union output_union
 * {
 *      BYTE midi_data[4];
 *      DWORD midi_out;
 * };
 *
 * a union to store the midi status and data bytes
 *
 * This union uses a DWORD which is a 32 bit integer
 * the diagram below shows how midi data is stored in a DWORD
 * 
 * 
 *    MSB   32 bit DWORD (midi_out)     LSB
 *      !------------------------------!
 *
 *                midi data array                                                          
 * 
 *         [3]     [2]     [1]     [0]
 *       data 2  data 1  data 0  status byte
 *      !------!!------!!------!!------!
 *         MSB     LSB     MSB     LSB
 * 
 * 
 * eg.  note on, channel 0, pitch 3 velocity 15
 *         [3]      [2]      [1]      [0]
 *      00000000 00001111 00000011 10010000  
 *      !------! !------! !------! !------!
 *
 * Note:- data 2 is set to zero as is not used in a note message.
 * 
 * Fill all unused bytes with zero.                                                             
 * 
 * Using a union will allow the direct accessing of each byte 
 * without any logical shifts and then the output DWORD can be accessed directly
 * for outputing to the midi device
 *
 */

static union output_union
{
	BYTE midi_data[4];
	DWORD midi_out;
};


/*
 *
 * struct midi_data -
 *
 * A structure to store the midi status and data bytes along with its relevant timestamp
 *
 * This structure is used to collect the driver timestamp and the processed
 * DWORD that windows returns containing midi message information 
 *
 */

struct midi_data
{
	unsigned long timestamp;
	BYTE type;
	BYTE channel;
	BYTE data1;
	BYTE data2;
	BYTE sysex[SYSEX_BUFF_SIZE];
	int bytes;
};

/*
 * a single static instance of the midi_data type which is used 
 * to buffer the incoming MIDI messages as they are retrieved
 * from the driver
 *
 */

static struct midi_data bufdata;


// two global variables to store references to the MIDI in and out devices, these are declared 
// static and are therefore only visible within the scope of this file

static HMIDIOUT __outHandle_       = NULL;
static HMIDIIN  __inHandle_        = NULL;
static MIDIHDR  __inputmidiHdr_, __outputmidiHdr_;	// Declare two buffers to be used by the system for sysex

static volatile int __tmdone_		= 0;		// Flag to indicate when a timing event has occured
static volatile int __messagedone_	= 0;		// Flag to indicate when a message is available

static __MIDI_in_initialised		= 0;		// Flag to indicate if MIDI input device is open
static __MIDI_out_initialised		= 0;		// Flag to indicate if MIDI output device is open
static __MIDI_recording			= 0;		// Flag to indicate if we're listening to MIDI input from the keyboard
static __Timing_initialised		= 0;		// Flag to indicate if High Res timing is on
static __atexit_registered		= 0;		// Flag to indicate if the atexit() function has been registered
static __CTR_Handler_registered		= 0;		// Flag to indicate if the ConsoleCtrlHandler function has been registered

#if(MIDI_CONFIG_METHOD == CONFIG_BY_DEVICE_DESC)
char *midi_output_dev_list[] = {MIDI_OUT_DEVICE, (char *)NULL};
char *midi_input_dev_list[] = {MIDI_IN_DEVICE, (char *)NULL};
#endif

// INTERNAL FUNCTION PROTOTYPES

BYTE    create_status_byte(int status,int channel);
BYTE    create_data_byte(int data);
void    output_midi(DWORD output);

void CALLBACK   midiCallback(HMIDIIN __inHandle_, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
void CALLBACK   TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
BOOL WINAPI     CtrlHandler(DWORD dwCtrlType);

void cleanup(void);

// FUNCTION DEFINITIONS

/*
 *      Function: void midi_note(int pitch, int channel, int velocity)
 *
 *      Send a note_on message on the requested channel with
 *      chosen pitch and velocity values.
 *
 */
 
void  midi_note(int pitch, int channel, int velocity)
{
	union output_union output;
	
	output.midi_out = 0;
	
	output.midi_data[0] = create_status_byte(NOTE_ON, channel);
	output.midi_data[1] = create_data_byte(pitch);
	output.midi_data[2] = create_data_byte(velocity);

	output_midi(output.midi_out);
}


/*
 *      Function: void program_change(int channel, int voice_number)
 *
 *      Send a program change message on the requested channel
 *      with the chosen voice number  
 *
 */

void  program_change(int channel, int voice_number)
{
	union output_union output;

	output.midi_out = 0;

	output.midi_data[0] = create_status_byte(PROG_CHANGE,channel);
	output.midi_data[1] = create_data_byte(voice_number - 1);

	output_midi(output.midi_out);
}


/*
 *      Function: void pitch_bend(int channel, int value)
 *
 *      Send a pitch bend message (only the MSB of 'value' is used here);
 *
 */

void  pitch_bend(int channel, int value)
{
	union output_union output;

	output.midi_out = 0;

	output.midi_data[0] = create_status_byte(PITCH_BEND,channel);

	// fill the next byte of the union with the required data byte
	// this is 2nd data byte and is pitch bend MSB (ignore LSB here)

	output.midi_data[2] = create_data_byte(value);

	output_midi(output.midi_out);
}


/*
 *      Function: void chan_pressure(int channel, int pressure)
 *
 *      Send a channel pressure message on the requested channel with pressure value.
 *
 */

void  chan_pressure(int channel, int pressure)
{
	union output_union output;

	output.midi_out = 0;

	output.midi_data[0] = create_status_byte(CHAN_PRESSURE, channel);
	output.midi_data[1] = create_data_byte(pressure);

	output_midi(output.midi_out);
}


/*
 *      Function: void polykey_pressure(int channel, int pitch, int value)
 *
 *      Send polykey pressure messages on the requested channel.  
 *
 */

void  polykey_pressure(int channel, int pitch, int value)
{
	union output_union output;

	output.midi_out = 0;

	output.midi_data[0] = create_status_byte(POLY_PRESSURE, channel);
	output.midi_data[1] = create_data_byte(pitch);
	output.midi_data[2] = create_data_byte(value);

	output_midi(output.midi_out);
}


/*
 *      Function: void midi_controller(int channel, int type, int value)
 *
 *      Send a controller message on the requested channel with
 *      chosen controller type and value.
 *
 */

void  midi_controller(int channel, int type, int value)
{
	union output_union output;

	output.midi_out = 0;

	output.midi_data[0] = create_status_byte(CONTROL_CHANGE, channel);
	output.midi_data[1] = create_data_byte(type);
	output.midi_data[2] = create_data_byte(value);

	output_midi(output.midi_out);
}


void bank_change(int channel, int bank, int voice_number)
{ 
	midi_controller(channel, 0x0,  bank >> 7);      // send Bank Select MSB
	midi_controller(channel, 0x20, bank & 0x0F);    // send Bank Select LSB

	program_change(channel, voice_number);          // change current program
}


/*
 *      Function: void send_NRPN(int channel, int msb, int lsb)
 *
 *      Sends an NRPN message.
 *
 */

void send_NRPN(int channel, int msb, int lsb)
{       
	midi_controller(channel, 0x63, msb);
	midi_controller(channel, 0x62, lsb);
}


/*
 *      Function: void send_RPN(int channel, int msb, int lsb)
 *
 *      Send an RPN message.
 *
 */

void send_RPN(int channel, int msb, int lsb)
{
	midi_controller(channel, 0x65, msb);
	midi_controller(channel, 0x64, lsb);
}


/*
 *      Function: void SysEx(int info_size, BYTE sysex_data[])
 *
 *      Send info_size bytes of SYSEX which is contained in the
 *      array sysex_data[].
 *
 *      The user must include the SYSEX head and tail data in
 *      this array themselves.
 *
 */

void SysEx(int info_size, BYTE sysex_data[])
{
	// create pointer to midi header
	LPMIDIHDR  header = &__outputmidiHdr_;
	
	if(!__MIDI_out_initialised)
	{
#if(AUTO_INITIALISE==1)
		open_midi_out_device();
#else
		printf("Error : No MIDI device open\n");
		abort();
#endif
	}
	
	__outputmidiHdr_.lpData = (LPBYTE) &sysex_data[0]; // Store pointer to our input buffer for SYSEX messages in MIDIHDR
	__outputmidiHdr_.dwBufferLength = info_size;       // Store its size in the MIDIHDR
	__outputmidiHdr_.dwFlags = 0;                      // Flags must be set to 0

	midiOutPrepareHeader(__outHandle_, header, sizeof(MIDIHDR));    // Prepare the buffer and MIDIHDR

	midiOutLongMsg(__outHandle_,header,info_size);    // output the passed SYSEX array.

	return;
}


/*
 *      Function: int get_midi_message(struct midi_data *msg)
 *
 *      Returns 1 if midi data is available.
 *      It places the available data into the user supplied 
 *      structure.  If the structure field, 'type' contains F 
 *      ie. SYSEX, then the 'bytes' field will contain the number 
 *      of bytes of SYSEX recorded and it will be place into the 
 *      'sysex' field of the structure.  Otherwise the data will 
 *      be used to fill the relevant data fields within the structure.
 *      Returns 0 if no data is available.
 *
 */

int get_midi_message(struct midi_data *msg)
{

	if(!__MIDI_in_initialised)
	{
#if(AUTO_INITIALISE==1)
		open_midi_in_device();
#else
		printf("Error get_midi_message() : No MIDI input device open.\n");
		abort();
#endif
	}

	// check to see if a message has been recieved by the device driver
	// this is stored in the flag __message_ done.

	if(__messagedone_) 
	{
		// set common message fields
		msg->timestamp  = bufdata.timestamp;
		msg->type       = bufdata.type;
		msg->channel    = bufdata.channel;

		// if the message is sysex then process it
		if((bufdata.type == SYS_EX) && (bufdata.channel == 0))
		{
			int loop;

			msg->bytes = bufdata.bytes;     // get the size of the recorded SYSEX
			msg->data1 = 0;
			msg->data2 = 0;

			// copy the SYSEX buffer into the users buffer
			for (loop = 0; loop < bufdata.bytes; loop++)
				msg->sysex[loop] = bufdata.sysex[loop];

			 
			__messagedone_ = 0;     // reset the messagedone flag as one has been processed
			return (1);             // notify the user the process is finished
		}       

		// if the MIDI message is a normal message then process it
		// fill in relevant data fields

		msg->data1      = bufdata.data1;
		msg->data2      = bufdata.data2;
		msg->channel    = bufdata.channel+1;
		msg->bytes      = 0;
		 
		__messagedone_  = 0;            // reset the messagedone flag as one has been processed
		return (1);                     // inform the user a message is available by returning a 1
	}
	else 
		return (0);                     // inform the user that no message has been recieved by returning a 0
}

/*
 *      Function: int get_midi_note(int *pitch, int *channel, int *velocity)
 *
 *      Function which returns 1 if a midi note is available
 *      and it places the note data into the user supplied pointers             
 *
 */

int get_midi_note(int *pitch, int *channel, int *velocity)
{
	if(!__MIDI_in_initialised)
	{
#if(AUTO_INITIALISE==1)
		open_midi_in_device();
#else
		printf("Error get_midi_note() : No MIDI input device open.\n");
		abort();
#endif
	}

	if(__messagedone_ && (bufdata.type == NOTE_ON)) 
	{       
		__messagedone_  = 0;
		*pitch          = bufdata.data1;
		*channel        = bufdata.channel + 1;
		*velocity       = bufdata.data2;
		return(1);
	}
	else 
		return(0);      // inform the user that no message has been recieved by returning a 0
}


/*
 *      Function: void pause(int millisecs)
 *
 *      Pause for 'millisecs' milliseconds and return.
 *
 */
 
void pause(int millisecs)
{
	PAUSE(millisecs);
	while(!PAUSE(millisecs));
}


/*
 *      Function: void clear_text_buffer(void)
 *
 *      Clear QWERTY keyboard buffer of any previously typed characters.  
 *
 */

void clear_text_buffer(void)
{
	while(kbhit() != 0) getch();
}


/*
 *      Function: short qwerty_input(void) 
 *
 *      Returns true if there is anything in QWERTY keyboard buffer.
 *
 */

short qwerty_input(void) 
{
	return((short)kbhit());
}


/*
 *      Function: char input_char(void) 
 *
 *      Get and returns a character from QWERTY keyboard - presuming one's there.
 *
 */

char input_char(void) 
{
	return((char)getch());
}


/*
 *      Function: int random_number(int minm, int maxm)
 *
 *      Returns a random integer the range minm <= n <= maxm
 *
 */

int random_number(int minm, int maxm)
{
	static int first_time = TRUE;   // flag to initialise random sequence generator
  
	int rand_val,randnum;
	float scale;

	if(first_time)
	{
		srand((unsigned) time(NULL));
		first_time = FALSE;
	}

	// create a random number between 0 and RAND_MAX
	// (RAND_MAX is defined in stdlib.h)

	rand_val = rand();

	scale = (float)rand_val/(float)RAND_MAX;        // create a floating point random number between 0 and 1
	randnum = (int)(minm+((maxm-minm)*scale));      // create a random int between minm and maxm

	return(randnum);
}


/*
 *      Function: int PAUSE(int millisecs)
 *
 *      pause function which triggers a timing event.
 *      The first time this function is called a timing event
 *      is triggered. Subsequent calls test to see if this event
 *      has occured yet. if this event has occured the function 
 *      returns 1 and resets the timer ready for another call.
 *
 */

int PAUSE(int millisecs)
{
	static int firsttime = 1;

	if(firsttime)
	{
		// create a one shot timing event which will set the __tmdone_ flag

		if(!__Timing_initialised)
		{
			timeBeginPeriod(1);
			__Timing_initialised = 1;
		}

		timeSetEvent(millisecs,1,TimeProc,(DWORD)__tmdone_,TIME_ONESHOT);

		firsttime = 0;  // set the first time flag to 0 so another timer is not triggered
		return (0);     // inform the user a timing event hasnt happened yet
	}
	else 
	{
		if(!__tmdone_)  // if it's not the first time check to see if timing event has occured
			return (0); 
		else 
		{
			firsttime = 1;  // if an event has occured reset the firsttime flag to 1
			__tmdone_ = 0;  // reset the timing flag
			return(1);      // return 1 to inform the user that an event has occured
		}
	}
}

#ifdef INCLUDE_DEVICE_INFO

/*
 *      Function: void device_info(void)
 *
 *      Simple utility function which will print to the console
 *      all available input and output MIDI devices
 *
 */

void device_info(void)
{
	MIDIOUTCAPS     moc;
	MIDIINCAPS      mic;
	TIMECAPS        tc;

	unsigned long iNumDevs, i;

	timeGetDevCaps(&tc, sizeof(tc));
	
	printf("Resolutions: minimun%d ms\n          : maximum%d ms\n\n", tc.wPeriodMin, tc.wPeriodMax);
	printf("Available output devices\n");

	// get number of devices
	iNumDevs = midiOutGetNumDevs();
	
	//next go through all devices and print their names and ID numbers
	for(i = 0; i < iNumDevs; i++)
	{
		if(!midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)))
			printf("Device ID #%u: %s\n", i, moc.szPname);
	}

	printf("\nAvailable input devices\n");

	iNumDevs = midiInGetNumDevs();

	//next go through all devices and print their names and ID numbers
	for(i = 0; i < iNumDevs; i++)
	{
		if(!midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)))
			printf("Device ID #%u: %s\n", i, mic.szPname);
	}
}
#endif

#if(MIDI_CONFIG_METHOD == CONFIG_BY_DEVICE_DESC)

/*
 *      Function: get_MIDI_device(int devtype, char *devlist[])
 *
 *      Search for devstring in MIDI input/output device list and return the device number.
 *
 */

unsigned long get_MIDI_device(int devtype, char *devlist[])
{
	MIDIOUTCAPS     moc;
	MIDIINCAPS      mic;

	unsigned long iNumDevs, i;
	int j;

	switch(devtype)
	{
		case MIDI_OUT:
			iNumDevs = midiOutGetNumDevs();
			break;

		case MIDI_IN:
			iNumDevs = midiInGetNumDevs();
			break;

		default:
			printf("get_MIDI_device() : Invalid device type specified : %d\n", devtype);
			abort();
	}

	for(j = 0; devlist[j] != (char *)NULL; j++)
	{
		switch(devtype)
		{
			case MIDI_OUT:
				for(i = 0; i < iNumDevs; i++)
					if(midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)) == 0 && strcmp(moc.szPname, devlist[j]) == 0)
						return i;
				break;

			case MIDI_IN:
				for(i = 0; i < iNumDevs; i++)
					if(midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)) == 0 && strcmp(mic.szPname, devlist[j]) == 0)
						return i;
				break;
		}	
	}

	printf("MIDI device config error : Failed to find %sput device from list :\n", ((devtype == MIDI_IN) ? "in" : "out"));

	for(j = 0; devlist[j] != (char *)NULL; j++)
	{
		printf("\t%s\n", devlist[j]);
	}

	abort();
}
#endif

/*
 *      Function: unsigned long getSystemtime(void)
 *
 *      Returns system time in milliseconds
 *
 */

unsigned long getSystemtime(void)
{
	static MMTIME mytime;   // A structure to hold the system time (MMTIME is defined by the windows API)

	timeGetSystemTime(&mytime, sizeof(mytime));     //get the system time.
	return (mytime.u.ms);                           // return its value in milliseconds
}


/*
 *      Function: int open_midi_in_device(void) & int open_midi_out_device(void)
 *
 *      Returns 0 if succesfull
 *
 *      These functions (formally a single open_midi_device function) MUST be called at the start of any 
 *      program wanting to use the MIDI devices. It calls the windows mmsystem functions to open input
 *      and output devices for midi and returns pointers to the midi input and output streams.It then
 *      prepares buffers for input and output of Sysex. It also registers a ctrl message handler for the
 *      console window which will detect the following messages
 *
 *      CTRL_C
 *      CTRL_BREAK
 *      close window
 *      user logging off
 *      shutdown
 *
 *      Any of these messages sent to this window will close it as per usual but will now close the midi ports too.
 *
 */

int open_midi_in_device(void)
{
	int err;
	
	if(__MIDI_in_initialised)
	{
		printf("Notice open_midi_in_device() : MIDI input device already open.\n");
		return 0;
	}

	if(!__atexit_registered)
	{
		__atexit_registered = 1;
		atexit(cleanup);
	}

	err = midiInOpen(&__inHandle_,
#if(MIDI_CONFIG_METHOD == CONFIG_BY_DEVICE_NUMBER)
	MIDI_IN_DEVICE,
#else
	get_MIDI_device(MIDI_IN, midi_input_dev_list),
#endif
	(DWORD)midiCallback, 0, CALLBACK_FUNCTION);

	if(!__Timing_initialised)
	{
		timeBeginPeriod(1);
		__Timing_initialised = 1;
	}

	if(!err)
	{
		// Store pointer to our input buffer for System Exclusive messages in MIDIHDR
		__inputmidiHdr_.lpData = (LPBYTE) &bufdata.sysex[0];

		// Store its size in the MIDIHDR
		__inputmidiHdr_.dwBufferLength = sizeof(bufdata.sysex);

		// Flags must be set to 0
		__inputmidiHdr_.dwFlags = 0;

		// Prepare the buffer and MIDIHDR
		err = midiInPrepareHeader(__inHandle_, &__inputmidiHdr_, sizeof(MIDIHDR));
		if(!err)
		{
			// Queue MIDI input buffer
			err = midiInAddBuffer(__inHandle_, &__inputmidiHdr_, sizeof(MIDIHDR));

			if(err)
				midiInUnprepareHeader(__inHandle_, &__inputmidiHdr_, sizeof(MIDIHDR));
			else
			{
				__MIDI_in_initialised = 1;

				// Try to add a new control handler to the console app. list.
				// This will catch all exit codes and try to close the MIDI ports
				// before leaving the app. - therefore not hanging it up.
				if(!__CTR_Handler_registered)
				{
					SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE);
					__CTR_Handler_registered = 1;
				}
				midi_in_start();
				pause(35);
			}
		}
	}

	return(err);
}

int open_midi_out_device(void)
{
	int err;
	
	if( __MIDI_out_initialised)
	{
		printf("Notice open_midi_device() : MIDI device already open.\n");
		return 0;
	}

	if(!__atexit_registered)
	{
		__atexit_registered = 1;
		atexit(cleanup);
	}

	// Open default MIDI Out device
	err = midiOutOpen(&__outHandle_,
#if(MIDI_CONFIG_METHOD == CONFIG_BY_DEVICE_NUMBER)
	MIDI_OUT_DEVICE,
#else
	get_MIDI_device(MIDI_OUT, midi_output_dev_list),
#endif
	0, 0, CALLBACK_NULL);


	if(!__Timing_initialised)
	{
		timeBeginPeriod(1);
		__Timing_initialised = 1;
	}

	if(!err)
	{
		__MIDI_out_initialised = 1;

		// Try to add a new control handler to the console app. list.
		// This will catch all exit codes and try to close the MIDI ports
		// before leaving the app. - therefore not hanging it up.

		if(!__CTR_Handler_registered)
		{
			SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE);
			__CTR_Handler_registered = 1;
		}
		pause(35);
	}

	return(err);
}


/*
 *      Function: void close_midi_in_device(void) & void close_midi_out_device(void)
 *
 *      These functions (formally a single close_midi_device function) MUST be called at
 *      the end of any program wanting to use the MIDI devices. They call the windows
 *      mmsystem functions to close currently open MIDI input or output devices.
 *
 *      Note :  If this function is not called before exiting the program or the program is 
 *              interrupted by the user eg. using Ctrl+C, the currently open midi devices will
 *              will not be available again within that windows session.                
 *
 */

void close_midi_in_device(void)
{
	if(!__MIDI_in_initialised)
		printf("Notice close_midi_in_device() : attempt to close unopened MIDI device.\n");
	else
	{
		if(__MIDI_recording)
			midi_in_stop();

		if(__inHandle_ != NULL)
		{
			midiInUnprepareHeader(__inHandle_, &__inputmidiHdr_, sizeof(MIDIHDR));
			midiInClose(__inHandle_);
		}

		__MIDI_in_initialised = 0;
	}
}

void close_midi_out_device(void)
{
	if(!__MIDI_out_initialised)
		printf("Notice close_midi_out_device() : attempt to close unopened MIDI device.\n");
	else
	{
		if(__outHandle_ != NULL)
		{
			midiOutUnprepareHeader(__outHandle_,&__outputmidiHdr_, sizeof(MIDIHDR));
			midiOutClose(__outHandle_);
		}

		__MIDI_out_initialised = 0;
	}
}

/*
 *      Function: void midi_in_start(void)
 *
 *      The midiInStart function starts MIDI input on the
 *      specified MIDI input device.
 *      This function resets the time stamp to zero; 
 *      time stamp values for subsequently received messages
 *      are relative to the time that this function was called.
 *
 */

void midi_in_start(void)
{
	if(!__MIDI_in_initialised)
	{
#if(AUTO_INITIALISE==1)
		open_midi_in_device();
#else
		printf("Error midi_in_start() : No MIDI device open\n");
		abort();
#endif
	}

	if(!__MIDI_recording)
		if(midiInStart(__inHandle_) == MMSYSERR_NOERROR)
		{
			__MIDI_recording = 1;
			return;
		}
		else
			return;
	else
		printf("Notice : midi_in_start called when already recording.\n");
}

/*
 *      Function: void midi_in_reset(void)
 *
 *      The midiInReset function stops & restarts input on a given MIDI input device.
 *
 */

void midi_in_reset(void)
{
	if(!__MIDI_in_initialised)
	{
		printf("Error midi_in_reset() : No MIDI device open\n");
		abort();
	}

	if(__MIDI_recording)
			return;
	else
		printf("Notice : midi_in_reset called when not recording.\n");
}


/*
 *      Function: void midi_in_stop(void)
 *
 *      The midiInStop function stops MIDI input on the specified MIDI input device.
 *
 */

void midi_in_stop(void)
{
	if(!__MIDI_in_initialised)
		printf("Notice midi_in_stop() : No MIDI device open\n");
	else
		if(__MIDI_recording)
			if(midiInStop(__inHandle_) == MMSYSERR_NOERROR)
			{
				__MIDI_recording = 0;
				return;
			}
			else
				return;
		else
			printf("Notice : midi_in_stop called when not recording.\n");
}


/*
 *      Function: BYTE create_status_byte(int status, int channel)
 *
 *      Simple function takes the status byte information as an int
 *      and also the channel number as an int. Checks both values.
 *      and converts them into two nibbles. It then OR's them into 1
 *      status byte.
 *
 *      eg. status  = Note_on   = 9 decimal = 00001001 binary
 *          channel = 1 decimal = 00000001 binary
 *      i.e. note_on message on channel 9 
 *
 *      First check both status and channel values and convert to unsigned 8-bit values
 *      next left shift status by 4 bits giving:1001000 binary
 *      next OR status and channel bytes giving final status byte.
 *
 *              stat            =       10010000
 *              chan            =       00000001
 *              chan OR stat    =       10010001 return this value
 *
 */

BYTE create_status_byte(int status, int channel)
{
	BYTE statbyte;

#if(CHECK_MODE==ON)
	BYTE stat, chan;

	// check status value is not < 8 and not > 15
	if(status > 16)
		status = 16;
	else if(status < 0)
		status = 0;
	
	stat = status << 4;     // assign value to temp byte

	// check channel value is not<1 or not>16
	if(channel > 16)
		channel = 16;
	else if(channel < 1)
		channel = 1;

	chan = channel - 1;     // assign value to temp byte

	statbyte = stat|chan;   // OR bytes together and return value

#else   
	statbyte = (status << 4)|((channel - 1) & 0x0F);
#endif

	return statbyte;
}


/*
 *      Function: BYTE create_data_byte(int data)
 *
 *      Formats 'data' so that its less than 127 and greater than 0.
 *
 */

BYTE create_data_byte(int data)
{
	BYTE dataout;

#if(CHECK_MODE==ON)
	// check data value is not < 0 and not > 127
	if(data > 127)
		data = 127;
	else if(data < 0)
		data = 0;

	dataout = (BYTE)data;

#else
	dataout = (BYTE)(data&0x7F);
#endif

	return dataout;
}


/*
 *      Function: void output_midi(DWORD output)
 *
 *      Output a MIDI message on the opened MIDI device.
 *
 */

void output_midi(DWORD output)
{
	if(!__MIDI_out_initialised)
	{
#if(AUTO_INITIALISE==1)
		if(open_midi_out_device() != 0)
		{
			printf("Error : Failed to open MIDI device for output.\n");
			abort();
		}
#else
		printf("Error : No MIDI device open\n");
		abort();
#endif
	}

	midiOutShortMsg(__outHandle_, output);
}

/*************************** midiCallback() *****************************
 *
 * callback that Windows calls whenever 1 of 4 possible things
 * happen:
 *
 * 1).  I open a MIDI In Device via midiInOpen(). In this case, the
 *      uMsg arg to my callback will be MIM_OPEN. The handle arg will
 *      be the same as what is returned from midiInOpen(). The
 *      dwInstance arg is whatever I passed to midiInOpen() as its
 *      dwInstance arg.
 *
 * 2).  I close a MIDI In Device via midiInClose(). In this case, the
 *      uMsg arg to my callback will be MIM_CLOSE. The handle arg will
 *      be the same as what was passed to midiInClose(). The
 *      dwInstance arg is whatever I passed to midiInClose() as its
 *      dwInstance arg when I initially opened this handle.
 *
 * 3).  One, regular (ie, everything except System Exclusive messages) MIDI
 *      message has been completely input. In this case, the uMsg arg to my
 *      callback will be MIM_DATA. The handle arg will be the same as what
 *      is passed to midiInOpen(). The dwInstance arg is whatever I passed
 *      to midiInOpen() as its dwInstance arg when I initially opened this
 *      handle. The dwParam1 arg is the bytes of the MIDI Message packed
 *      into an unsigned long in the same format that is used by
 *      midiOutShort(). The dwParam2 arg is a time stamp that the device
 *      driver created when it recorded the MIDI message.
 *
 * 4).  midiInOpen has either completely filled a MIDIHDR's memory buffer
 *      with part of a System Exclusive message (in which case we had better
 *      continue queuing the MIDIHDR again in order to grab the remainder
 *      of the System Exclusive), or the MIDIHDR's memory buffer contains the
 *      remainder of a System Exclusive message (or the whole message if it
 *      happened to fit into the memory buffer intact). In this case, the
 *      uMsg arg to my callback will be MIM_LONGDATA. The handle arg will be
 *      the same as what is passed to midiInOpen(). The dwInstance arg is
 *      whatever I passed to midiInOpen() as its dwInstance arg when I
 *      initially opened this handle. The dwParam1 arg is a pointer to the
 *      MIDIHDR whose memory buffer contains the System Exclusive data. The
 *      dwParam2 arg is a time stamp that the device driver created when it
 *      recorded the MIDI message.
 *
 * 5).  This callback is not processing data fast enough such that the MIDI
 *      driver (and possibly the MIDI In port itself) has had to throw away
 *      some incoming, regular MIDI messages. In this case, the uMsg arg to my
 *      callback will be MIM_MOREDATA. The handle arg will be the same as what
 *      is passed to midiInOpen(). The dwInstance arg is whatever I passed
 *      to midiInOpen() as its dwInstance arg when I initially opened this
 *      handle. The dwParam1 arg is the bytes of the MIDI Message that was
 *      not handled (by an MIM_DATA call) packed into an unsigned long in the
 *      same format that is used by midiOutShort(). The dwParam2 arg is a time
 *      stamp that the device driver created when it recorded the MIDI message.
 *      In handling a series of these events, you should store the MIDI data
 *      in a global buffer, until such time as you receive another MIM_DATA
 *      (which indicates that you can now do the more time-consuming processing
 *      that you obviously were doing in handling MIM_DATA).
 *      NOTE: Windows sends an MIM_MOREDATA event only if you specify the
 *      MIDI_IO_STATUS flag to midiInOpen().
 *
 * 6).  An invalid, regular MIDI message was received. In this case, the uMsg
 *      arg to my callback will be MIM_ERROR. The handle arg will be the same
 *      as what is passed to midiInOpen(). The dwInstance arg is whatever I
 *      passed to midiInOpen() as its dwInstance arg when I initially opened
 *      this handle. The dwParam1 arg is the bytes of the MIDI Message that was
 *      not handled (by an MIM_DATA call) packed into an unsigned long in the
 *      same format that is used by midiOutShort(). The dwParam2 arg is a time
 *      stamp that the device driver created when it recorded the MIDI message.
 *
 * 7).  An invalid, System Exclusive message was received. In this case, the uMsg
 *      arg to my callback will be MIM_LONGERROR. The handle arg will be the same
 *      as what is passed to midiInOpen(). The dwInstance arg is whatever I
 *      passed to midiInOpen() as its dwInstance arg when I initially opened
 *      this handle. The dwParam1 arg is a pointer to the MIDIHDR whose memory
 *      buffer contains the System Exclusive data. The dwParam2 arg is a time
 *      stamp that the device driver created when it recorded the MIDI message.
 *
 * The time stamp is expressed in terms of milliseconds since your app
 * called midiInStart().
 *
 ************************************************************************/

void CALLBACK midiCallback(HMIDIIN __inHandle_, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	LPMIDIHDR       lpMIDIHeader;
	unsigned char  *ptr;
    
	
	/* Determine why Windows called me */

	switch(uMsg)
	{
		// Received some regular MIDI message
		case MIM_DATA:
			// timestamp the message structure (dwParam2 contains the timestamp)
			bufdata.timestamp = dwParam2;

			// dwParam1 contains all of the other information
			bufdata.type    = (BYTE) ((dwParam1        & 0x000000F0) >> 4);
			bufdata.channel = (BYTE) ( dwParam1        & 0x0000000F);
			bufdata.data1   = (BYTE) ((dwParam1 >>  8) & 0x000000FF);
			bufdata.data2   = (BYTE) ((dwParam1 >> 16) & 0x000000FF);
			
			// set the message flag to indicate a message is available.
			__messagedone_ = 1;
			break;

		// Received SYSEX
		case MIM_LONGDATA:
			// timestamp the message structure.
			bufdata.timestamp = dwParam2;

			// Assign address of MIDIHDR to a LPMIDIHDR variable. Makes it easier to access the
			// field that contains the pointer to our block of MIDI events.
			lpMIDIHeader = (LPMIDIHDR)dwParam1;

			// Get address of the MIDI event that caused this call.
			ptr = (unsigned char *) (lpMIDIHeader->lpData);

			// Store the number of bytes recorded into the bytes element of bufdata.
			bufdata.bytes = lpMIDIHeader->dwBytesRecorded;

			// Fill the data and channel type elements with the status byte.
			bufdata.type = ((*ptr)&0xF0)>>4;
			bufdata.channel = ((*ptr)&0x0F);
						
			// Queue the MIDIHDR for more input.
			midiInAddBuffer(__inHandle_, lpMIDIHeader, sizeof(MIDIHDR));
			
			// Set the message flag to indicate a mesage is available.
			__messagedone_ = 1;
			break;
			
		// Process other messages.

		case MIM_OPEN:          break;
		case MIM_CLOSE:         break;
		case MIM_ERROR:         break;
		case MIM_LONGERROR:     break;
		case MIM_MOREDATA:      break;
		default : break;
	}       
}


/*
 *      Function: void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
 *
 *      Timer call back function.
 *
 */

void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	__tmdone_ = 1;  // set the timer event flag
	return;
}


/*
 *      Function: BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) 
 *
 *      Catch and process window closing and user requests to close the program.
 *      It tries to close the MIDI ports prior to exiting the program on window's
 *      or the users request.
 *
 */

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) 
{ 
	switch (fdwCtrlType) 
	{ 
		// Handle the CTRL+C signal. 

		case CTRL_C_EVENT: 
		// Pass other signals to the next handler. 

		case CTRL_CLOSE_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:

		default:
			// close devices and return false
			// This allows windows to process the rest of the closing event

			cleanup();
			return FALSE; 
	} 
} 

void cleanup(void)
{
	if(__MIDI_in_initialised)
		close_midi_in_device();

	if(__MIDI_out_initialised)
		close_midi_out_device();

	if(__Timing_initialised)
	{
		timeEndPeriod(1);
		__Timing_initialised = 0;
	}
}
