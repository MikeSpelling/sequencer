#ifndef __MIDI_LIBRARY_H__
#define __MIDI_LIBRARY_H__

/*************************************************************************/
/*                                                                       */
/*      MIDI C library                                                   */
/*      ==============                                                   */
/*                                                                       */
/*      Original library for use on SGI by : Dr Andy Hunt                */
/*                                                                       */
/*      Adapted and expanded to use Roland SoundCanvas under MS-DOS      */
/*                                      by : Mr Terry Edhouse            */
/*                                                                       */
/*      Adapted to work under MS-Windows 9x/NT/2000 with any MIDI        */
/*      compatible soundcard            by : Mr Luke Geldart             */
/*                                                                       */
/*      Adapted to compile under Microsoft Visual C++                    */
/*                                      by : Dr Steve Smith              */
/*                                                                       */
/*      Compiled to library file (code removed from header)              */
/*                                      by : Mr Peter Mendham            */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/* Note : This header file is set-up specifically to drive the           */
/*        soundcards in both the Electronics PC lab systems and the      */
/*        campus classroom PCs.                                          */
/*                                                                       */
/*        While it may run on your home system without needing any       */
/*        alterations it's possible that the values of MIDI_OUT_DEVICE   */
/*        and MIDI_IN_DEVICE may need changing.                          */
/*                                                                       */
/*        Future versions of this library will provide a device          */
/*        configuration facility to simplify the set-up process.         */
/*                                                                       */
/*        If you take a copy of this library for use on your home system */
/*        please check for updates regularly.                            */
/*                                                                       */
/*************************************************************************/

/*
 * Important MIDI :     THIS FILE MUST BE LINKED TO WINMM.LIB ????? NO LONGER TRUE ?????
 *                      THIS IS TRUE FOR COMPILATION UNDER MICROSOFT VISUAL C++ COMMAND LINE
 *                      if this is not done your compiler will complain about all
 *                      of the Windows multimedia functions used within this file.
 *
 *                      To compile using Borland C CLI use :
 *
 *                              bcc32 source.c
 *
 *                      replacing source.c with the name of your C source file.
 *
 *
 *                      To compile using Microsoft Visual C++ command line use :
 *
 *                              cl source.c winmm.lib
 *
 *                      replacing source.c with the name of your C source file.
 *
 *           Note :     Valid MIDI channels are 1-16
 *                      Valid programs are 0-127
 */

#define _MIDI_LIB_VERSION_ "5.01"

/********************************************************************************************************/
/*      VERSION HISTORY                                                                                 */
/*      ===============                                                                                 */
/*                                                                                                      */
/*      3.00    : 21-SEP-2000 : Initial Windows 9x/NT/2000 version.                                     */
/*                                                                                                      */
/*      3.01    : 17-OCT-2000 : Add MIDI_CONFIG_METHOD to allow specification of the configuration      */
/*                              method to be used.                                                      */
/*                                                                                                      */
/*                              Add get_MIDI_device() to search for the strings defined by              */
/*                              MIDI_IN_DEVICE & MIDI_OUT_DEVICE in the MIDI device list, and return    */
/*                              the MIDI input & output device numbers accordingly.                     */
/*                                                                                                      */
/*      3.02    : 17-NOV-2000 : Make random number initialisation more random.                          */
/*                                                                                                      */
/*      3.03    : 18-MAY-2001 : Register close_midi_device() as an 'atexit' function to ensure          */
/*                              everything is properly shutdown when the user's program terminates.     */
/*                                                                                                      */
/*                              NOTE : Calling abort() or _exit() will cause the atexit chain to be     */
/*                                     bypassed.                                                        */
/*                                                                                                      */
/*      3.04    : 28-MAY-2001 : Adjust value of voice number passed to program_change(), by -1.         */
/*                              Initialise MIDI input in open_midi_device() to overcome note loss and   */
/*                              to prevent program_change() from breaking MIDI input.                   */
/*                                                                                                      */
/*                              Add cleanup() and use as the atexit() function for better shutdown      */
/*                              control.                                                                */
/*                                                                                                      */
/*                              Make close_midi_device() check that handles etc are initialised before  */
/*                              using them.                                                             */
/*                                                                                                      */
/*                              Make open_midi_device() tidy-up if errors are encountered.              */
/*                                                                                                      */
/*                              Correct confusion over __outputmidiHdr_ & __inputmidiHdr_.              */
/*                                                                                                      */
/*      3.05	: 05-OCT-2001 : Change default MIDI device to SB Live! for new PC lab systems         */
/*                              Correct device type test in get_MIDI_device()                           */
/*                                                                                                      */
/*      4.00	: 23-JUL-2003 : Change MIDI_IN_DEVICE & MIDI_OUT_DEVICE to string list to allow a     */
/*                              list of devices to be specified.  When initialising, each entry in the  */
/*                              list will be tried until a match is found.  This will allow a program   */
/*                              compiled on a campus PC to run on either a campus PC or in our lab.     */
/*                                                                                                      */
/*                              Split the open_midi_device function into open_midi_in_device &          */
/*                              open_midi_out_device.  Same with close_midi_device.                     */
/*                                                                                                      */
/*      5.00      : 19-AUG-2003 : Alterations to compile under Microsoft C++ command line compiler      */
/*                              "cl" ver 12.  Commented out Borland compiler warning suppression        */
/*                              "#pragma warn -???".  Replaced Borland specif function "randomize()"    */
/*                              with "srand((unsigned) time(NULL))".  Removed return values from        */
/*                              functions "midi_in_start()", "midi_in_reset()" and "midi_in_stop()" and */
/*                              made void to prevent warning messages indicating that not all control   */
/*                              paths return a value.  Externally named "midi_lib.h" to avoid confusion */
/*                              with version used in Electronics Lab under Borland C++.                 */
/*                                                                                                      */
/*      5.01	: 23-SEP-2005 : Compiled into library file rather than leaving code in header file      */
/*                              to allow use within multiple source files (correct linking).            */
/*                                                                                                      */
/********************************************************************************************************/

#include <stdlib.h>

#define BYTE unsigned char
#define OFF 0
#define ON 1
#define MIDI_IN  0
#define MIDI_OUT 1

#define INCLUDE_DEVICE_INFO

#define CONFIG_BY_DEVICE_NUMBER         0
#define CONFIG_BY_DEVICE_DESC           1

// #define CONFIG_BY_USER_SELECTION     2  // Not currently implemented

/*
 * CONFIGURATION SETTINGS	-- To override the defaults in this header file insert #define's into your C source file BEFORE you #include this file.
 *
 * There are currently two ways of selecting the MIDI input & output devices to use in your program.
 * To choose the method you wish to use, #define the variable MIDI_CONFIG_METHOD to either :
 *
 *      CONFIG_BY_DEVICE_NUMBER or CONFIG_BY_DEVICE_DESC
 *
 *
 * 	CONFIG_BY_DEVICE_NUMBER :
 *
 * 	To use this configuration method, set the #define variables MIDI_OUT_DEVICE and MIDI_IN_DEVICE to the
 * 	device numbers you wish to use as listed by the device_info() function.
 * 	(#define INCLUDE_DEVICE_INFO to include device_info() function).
 *
 * 	While this is the most efficient method, it is rather inflexible.
 *
 *
 * 	CONFIG_BY_DEVICE_DESC :
 *
 * 	To use this configuration method, set the #define variables MIDI_OUT_DEVICE and MIDI_IN_DEVICE to comma seperated lists of
 * 	strings corresponding to the set of acceptable MIDI devices - in order of preference.
 *
 * 	The device_info() function displays descriptions of the devices available on the PC it is run on.
 *
 * 	(#define INCLUDE_DEVICE_INFO to include device_info() function).
 *
 * 	Uses the get_MIDI_device() function to search the MIDI device list for each of the devices specified in
 * 	MIDI_OUT_DEVICE/MIDI_IN_DEVICE and returns the device number.
 *
 * 	While not as efficient as the CONFIG_BY_DEVICE_NUMBER method, this method will find the correct device
 * 	(if it exists in the machine the program is running on) regardless of it's position in the MIDI device list.
 *
 */

#define MIDI_CONFIG_METHOD CONFIG_BY_DEVICE_DESC

#ifndef MIDI_OUT_DEVICE
#define MIDI_OUT_DEVICE "A: SB Live! MIDI Synth", "Microsoft GS Wavetable SW Synth"
#endif

#ifndef MIDI_IN_DEVICE
#define MIDI_IN_DEVICE "SB Live! MIDI In"
#endif


/*
 * AUTO_INITIALISE :
 *
 *      If AUTO_INITIALISE is set to 1 then the first call to a MIDI function will
 *      automatically call open_midi_device()
 *
 *      If AUTO_INITIALISE is set to 0 then the user must explicitly call open_midi_device()
 *      before using any of the MIDI functions.
 *
 */

#ifndef AUTO_INITIALISE
#define AUTO_INITIALISE 1
#endif


/*
 * CHECK_MODE :
 *
 *      If CHECK_MODE is ON then the library will perform the bounds
 *      checking for data ie data > 0, data < 127 :-
 *
 *      if data < 0     then data = 0
 *      if data > 127   then data = 127
 *
 *      if CHECK_MODE is OFF then the data is just formatted with the
 *      appropraiate binary mask. This will make sure it is 127 or less.
 *
 */

#ifndef CHECK_MODE
#define CHECK_MODE ON
#endif


#define SYSEX_BUFF_SIZE 1024
#define MIDI_IN_BUFF_SIZE 100


// DEFINE MIDI STATUS NIBBLES

#define NOTE_OFF        0x8     // 1000
#define NOTE_ON         0x9     // 1001
#define POLY_PRESSURE   0xA     // 1010
#define CONTROL_CHANGE  0xB     // 1011
#define PROG_CHANGE     0xC     // 1100
#define CHAN_PRESSURE   0xD     // 1101
#define PITCH_BEND      0xE     // 1110
#define SYS_EX          0xF     // 1111

// FUNCTION PROTOTYPES DEFINED HERE

// DECLARE USER MIDI FUNCTIONS

// MIDI output functions

void    midi_note(int pitch, int channel, int velocity);
void    program_change(int channel, int voice_number);
void    pitch_bend(int channel, int value);
void    chan_pressure(int channel, int pressure);
void    polykey_pressure(int channel, int pitch, int value);
void    midi_controller(int channel, int type, int value);
void    bank_change(int channel, int bank, int voice_number);
void    send_NRPN(int channel, int msb, int lsb);
void    send_RPN(int channel, int msb, int lsb);
void    SysEx(int info_size, BYTE sysex_data[]);

// MIDI input functions

int     get_midi_message(struct midi_data *msg);
int     get_midi_note(int *pitch, int *channel, int *velocity);

// USER UTILITY FUNCTIONS

void    pause(int millisecs);
void    clear_text_buffer(void);
short   qwerty_input(void);
char    input_char(void);
int     random_number(int minm, int maxm);

int     PAUSE(int millisecs);

#ifdef INCLUDE_DEVICE_INFO
void    device_info(void);
#endif

#if(MIDI_CONFIG_METHOD == CONFIG_BY_DEVICE_DESC)
unsigned long   get_MIDI_device(int devtype, char *devlist[]);
#endif

unsigned long   getSystemtime(void);

// MIDI device control

int     open_midi_in_device(void);
int     open_midi_out_device(void);
void    close_midi_in_device(void);
void    close_midi_out_device(void);

// MIDI input control

void     midi_in_start(void);
void     midi_in_reset(void);
void     midi_in_stop(void);

#endif
