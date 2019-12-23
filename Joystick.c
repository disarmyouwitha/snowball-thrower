/*
Nintendo Switch Fightstick - Proof-of-Concept

Based on the LUFA library's Low-Level Joystick Demo
	(C) Dean Camera
Based on the HORI's Pokken Tournament Pro Pad design
	(C) HORI

This project implements a modified version of HORI's Pokken Tournament Pro Pad
USB descriptors to allow for the creation of custom controllers for the
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
the Pokken Controller from functioning at the same level as the Pro
Controller. However, by default most of the descriptors are there, with the
exception of Home and Capture. Descriptor modification allows us to unlock
these buttons for our use.
*/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "uart.h"
#include "Joystick.h"
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define BAUD_RATE 9600

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

// uart_putchar('!');

// struct for action_list array:
typedef struct action_item { char _char; float _time; float _held; } action_item;

// [Define some global buttons]:
typedef enum {
    Y,
    NOTHING,
    TRIGGERS,
    READ_INPUT
} Buttons_t;

typedef struct {
	Buttons_t button;
	uint16_t duration;
} command;

// [Bot Instructions]:
static const command step[] = 
{
    // [Setup controller[:]
    { NOTHING,      250 },
    { TRIGGERS,       5 },
    { NOTHING,        5 }, // I was getting splat-bombs thrown after controller sync so I think it was L+R too many times.
    { NOTHING,        5 }, // { TRIGGERS,   5 },
    { NOTHING,        5 }, // { NOTHING,  150 },
    { Y,             10 }, // { A,          5 },
    { NOTHING,        5 }, // { NOTHING,  250 },
    // ^(Running out room on instructions until index 7)

    // [Bot Instructions]:
    { READ_INPUT,   200 }
};

// Set up some booleans for HELD state:
int MOUSE_SPEED = 0;
bool W_bool = false;
bool A_bool = false;
bool S_bool = false;
bool D_bool = false;
bool E_bool = false;
bool F_bool = false; int _SQUID_OR_WALK_ = 0;
bool J_bool = false;
bool R_bool = false;
bool I_bool = false;
bool K_bool = false;
bool Q_bool = false;
bool U_bool = false;
bool M_bool = false;
bool _CLEAR_ = false;
bool B_button = false;
bool A_button = false;
bool SPACE_bool = false;
bool BOOYA_bool = false;
bool _BOT_REPLAY_ = true;

// HATS for left buttons (BOOYA, etc)
void set_bool(char c)
{
    // Set ONs:
    if (c == 'W') { W_bool = true; }
    if (c == 'A') { A_bool = true; }
    if (c == 'S') { S_bool = true; }
    if (c == 'D') { D_bool = true; }
    if (c == 'E') { E_bool = true; }
    if (c == 'F') { _SQUID_OR_WALK_ = ~_SQUID_OR_WALK_; }
    if (c == 'J') { J_bool = true; }
    if (c == 'R') { R_bool = true; }
    if (c == 'I') { I_bool = true; }
    if (c == 'K') { K_bool = true; }
    if (c == 'Q') { Q_bool = true; }
    if (c == 'U') { U_bool = true; }
    if (c == 'M') { M_bool = true; }
    if (c == 'G') { A_button = true; }
    if (c == 'H') { B_button = true; }
    if (c == ' ') { SPACE_bool = true;}

    // Set OFFs:
    if (c == 'w') { W_bool = false; }
    if (c == 'a') { A_bool = false; }
    if (c == 's') { S_bool = false; }
    if (c == 'd') { D_bool = false; }
    if (c == 'e') { E_bool = false; }
    //if (c == 'f') { F_bool = false; }
    if (c == 'j') { J_bool = false; }
    if (c == 'r') { R_bool = false; }
    if (c == 'i') { I_bool = false; }
    if (c == 'k') { K_bool = false; }
    if (c == 'q') { Q_bool = false; }
    if (c == 'u') { U_bool = false; }
    if (c == 'm') { M_bool = false; }
    if (c == 'g') { A_button = true; }
    if (c == 'h') { B_button = true; }

    // Set MOUSE_SPEED:
    if (c == '1') { MOUSE_SPEED = 1; }
    if (c == '2') { MOUSE_SPEED = 2; }
    if (c == '3') { MOUSE_SPEED = 3; }
    if (c == '4') { MOUSE_SPEED = 4; }
    if (c == '5') { MOUSE_SPEED = 5; }
    if (c == '6') { MOUSE_SPEED = 6; }
    if (c == '7') { MOUSE_SPEED = 7; }
    if (c == '8') { MOUSE_SPEED = 8; }
    if (c == '9') { MOUSE_SPEED = 9; }
}

uint8_t incomingByte = 0;

// [Main entry point]:
int main(void)
{
    SetupHardware();            // We'll start by performing hardware and peripheral setup.
    GlobalInterruptEnable();    // We'll then enable global interrupts for our use.


    // [Once that's done, we'll enter an infinite loop]:
    for (;;)
    {
        HID_Task();     // We need to run our task to process and deliver data for our IN and OUT endpoints.
        USB_USBTask();  // We also need to run the main USB management task.

        // [Serial communication]:
        if (uart_available()) 
        {
            incomingByte = uart_getchar();
            set_bool(incomingByte);
        } else {
            incomingByte = 0;
        }
    }
}

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) 
{
    // We need to disable watchdog if enabled by bootloader/fuses.
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    // We need to disable clock division before initializing the USB hardware.
    clock_prescale_set(clock_div_1);

    CPU_PRESCALE(0);  // run at 16 MHz
    uart_init(BAUD_RATE);

    // The USB stack should be initialized last.
    USB_Init();
}

// [Fired to indicate that the device is enumerating]:
void EVENT_USB_Device_Connect(void)
{
    // We can indicate that we're enumerating here (via status LEDs, sound, etc.)
}

// [Fired to indicate that the device is no longer connected to a host]:
void EVENT_USB_Device_Disconnect(void)
{
    // We can indicate that our device is not ready (via status LEDs, sound, etc.)
}

// [Fired when the host set the current configuration of the USB device after enumeration]:
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    // We setup the HID report endpoints.
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

    // We can read ConfigSuccess to indicate a success or failure at this point.
}

// [Process control requests sent to the device from the USB host]:
void EVENT_USB_Device_ControlRequest(void)
{
    // We can handle two control requests: a GetReport and a SetReport.
    // Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
    // [If the device isn't connected and properly configured, we can't do anything here]:
    if (USB_DeviceState != DEVICE_STATE_Configured)
        return;

    // [We'll start with the OUT endpoint]:
    Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
    if (Endpoint_IsOUTReceived()) // We'll check to see if we received something on the OUT endpoint:
    {
        // If we did, and the packet has data, we'll react to it.
        if (Endpoint_IsReadWriteAllowed())
        {
            USB_JoystickReport_Output_t JoystickOutputData; // We'll create a place to store our data received from the host.

            // We'll then take in that data, setting it up in our storage.
            //while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
            Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL);

            // At this point, we can react to this data.
            // However, since we're not doing anything with this data, we abandon it.
        }
        // Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
        Endpoint_ClearOUT();
    }

    // [We'll then move on to the IN endpoint]:
    Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
    if (Endpoint_IsINReady()) // We first check to see if the host is ready to accept data:
    {
        USB_JoystickReport_Input_t JoystickInputData;   // We'll create an empty report.
        GetNextReport(&JoystickInputData);              // We'll then populate this report with what we want to send to the host.

        // Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
        //while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
        Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL);

        // We then send an IN packet on this endpoint.
        Endpoint_ClearIN();
    }
}

// [DEFINE STATE]:
typedef enum {
    BOT_INSTRUCTIONS,
    SYNC_CONTROLLER,
    SYNC_POSITION,
    BREATHE,
    PROCESS,
    CLEANUP,
    DONE, 
} State_t;
State_t state = SYNC_CONTROLLER;

int _cnt = 0;
int echoes = 0;
int bufindex = 0;
bool _DONE = false;
int _action_length;
int report_count = 0;
int command_count = 0;
int duration_count = 0;
struct action_item action_list[19];
struct action_item release_list[19];
USB_JoystickReport_Input_t last_report;

#define ECHOES 2
#define max(a, b) (a > b ? a : b)
#define ms_2_count(ms) (ms / ECHOES / (max(POLLING_MS, 8) / 8 * 8))

// [Prepare the next report for the host]:
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) 
{
    // [Prepare an empty report]:
    memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
    ReportData->LX = STICK_CENTER;
    ReportData->LY = STICK_CENTER;
    ReportData->RX = STICK_CENTER;
    ReportData->RY = STICK_CENTER;
    ReportData->HAT = HAT_CENTER;

    // Repeat ECHOES times the last report
    if (echoes > 0)
    {
        memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
        echoes--;
        return;
    }

    // BOT_REPLAY
    if (_BOT_REPLAY_)
    {
        switch (state)
        {
            case SYNC_CONTROLLER:
                if (command_count > ms_2_count(2000))
                {
                    command_count = 0;
                    state = BOT_INSTRUCTIONS;
                    uart_putchar('B');
                    //-------
                    _action_length = 19;
                    //struct action_item action_list[19];
                    action_list[0]= (action_item){ 'W', 3.459, 1.373 };
                    action_list[1]= (action_item){ 'D', 5.760, 0.592 };
                    action_list[2]= (action_item){ 'A', 7.184, 0.216 };
                    action_list[3]= (action_item){ 'A', 10.712, 0.096 };
                    action_list[4]= (action_item){ 'W', 11.104, 0.496 };
                    action_list[5]= (action_item){ 'W', 12.000, 0.224 };
                    action_list[6]= (action_item){ 'J', 10.144, 2.896 };
                    action_list[7]= (action_item){ 'A', 13.448, 2.064 };
                    action_list[8]= (action_item){ 'W', 15.232, 1.424 };
                    action_list[9]= (action_item){ 'W', 17.288, 0.408 };
                    action_list[10]= (action_item){ 'J', 13.832, 4.064 };
                    action_list[11]= (action_item){ 'W', 18.024, 0.232 };
                    action_list[12]= (action_item){ 'D', 18.448, 0.496 };
                    action_list[13]= (action_item){ 'W', 19.016, 1.672 };
                    action_list[14]= (action_item){ 'J', 18.296, 2.888 };
                    action_list[15]= (action_item){ 'S', 21.168, 0.080 };
                    action_list[16]= (action_item){ 'F', 21.496, 0.088 };
                    action_list[17]= (action_item){ 'S', 21.880, 2.440 };
                    action_list[18]= (action_item){ 'F', 27.496, 0.072 };
                    //---
                    //struct action_item release_list[19];
                    release_list[0]= (action_item){ '~', 0, 0 };
                    release_list[1]= (action_item){ '~', 0, 0 };
                    release_list[2]= (action_item){ '~', 0, 0 };
                    release_list[3]= (action_item){ '~', 0, 0 };
                    release_list[4]= (action_item){ '~', 0, 0 };
                    release_list[5]= (action_item){ '~', 0, 0 };
                    release_list[6]= (action_item){ '~', 0, 0 };
                    release_list[7]= (action_item){ '~', 0, 0 };
                    release_list[8]= (action_item){ '~', 0, 0 };
                    release_list[9]= (action_item){ '~', 0, 0 };
                    release_list[10]= (action_item){ '~', 0, 0 };
                    release_list[11]= (action_item){ '~', 0, 0 };
                    release_list[12]= (action_item){ '~', 0, 0 };
                    release_list[13]= (action_item){ '~', 0, 0 };
                    release_list[14]= (action_item){ '~', 0, 0 };
                    release_list[15]= (action_item){ '~', 0, 0 };
                    release_list[16]= (action_item){ '~', 0, 0 };
                    release_list[17]= (action_item){ '~', 0, 0 };
                    release_list[18]= (action_item){ '~', 0, 0 };
                    //---
                }
                else
                {
                    if (command_count == ms_2_count(500) || command_count == ms_2_count(1000))
                    {
                        uart_putchar('L');
                        ReportData->Button |= SWITCH_L | SWITCH_R;
                    } else if (command_count == ms_2_count(1500) || command_count == ms_2_count(2000)) {
                        ReportData->Button |= SWITCH_A;
                        uart_putchar('A');
                    }
                    command_count++;
                }
                break;
            case BOT_INSTRUCTIONS:
                if (_DONE)
                {
                    command_count = 0;
                    state = DONE;
                    uart_putchar(' ');
                } else {
                    // [Main loop]:
                    if(_cnt<_action_length) 
                    {
                        action_item _action = action_list[_cnt];
                        if (command_count >= ms_2_count(_action._time*1000))
                        {
                            set_bool(_action._char);
                            uart_putchar(_action._char);
                            release_list[_cnt]= (action_item){ tolower(_action._char), (_action._time+_action._held), 0 };
                            _cnt++;
                        }
                    }

                    // Hold for (_time+_held) / release with:
                    for(int k; k<_action_length; k++)
                    {
                        action_item _curr = release_list[k];
                        if(_curr._char != '~')
                        {
                            if (command_count >= ms_2_count(_curr._time*1000))
                            {
                                uart_putchar(tolower(_curr._char));
                                set_bool(tolower(_curr._char));
                                release_list[k]._char = '~';

                                // [Last step+delay]:
                                if (k==_action_length-1)
                                {
                                    _DONE = true;
                                }
                            }
                        }
                    }
                }

                // [Command list]:
                if (A_button) { ReportData->Button |= SWITCH_A; }
                if (B_button) { ReportData->Button |= SWITCH_B; }
                if(W_bool) { ReportData->LY = STICK_MIN; } // FORWARD
                if(A_bool) { ReportData->LX = STICK_MIN; } // LEFT
                if(S_bool) { ReportData->LY = STICK_MAX; } // BACKWARD
                if(D_bool) { ReportData->LX = STICK_MAX; } // RIGHT:
                if(J_bool) { ReportData->Button |= SWITCH_ZR; } // FIRE!
                if(U_bool) { ReportData->Button |= SWITCH_X; } // U (Map, x)
                if(I_bool) { ReportData->Button |= SWITCH_R; } // I (SUB)
                if(K_bool) { ReportData->Button |= SWITCH_RCLICK; } // M (SPECIAL)
                if(R_bool) { ReportData->Button |= SWITCH_Y; } // R (Reset camera,y)
                if(M_bool) { ReportData->Button |= SWITCH_A; } // CONFIRM (A)
                if(Q_bool) { ReportData->RX = STICK_MIN; } // LOOK LEFT
                if(E_bool) { ReportData->RX = STICK_MAX; } // LOOK RIGHT
                if(_SQUID_OR_WALK_) { ReportData->Button |= SWITCH_ZL; } // SQUID OR WALK: (F)
                if(_CLEAR_) 
                {
                    ReportData->LX = STICK_CENTER;
                    ReportData->LY = STICK_CENTER;
                    ReportData->RX = STICK_CENTER;
                    ReportData->RY = STICK_CENTER;
                    ReportData->HAT = HAT_CENTER;
                }
                command_count++;
                break;
            case DONE:
                break;
        }
    } else {
        // BOT_RECORD
        switch (state)
        {
            case SYNC_CONTROLLER:
                state = BREATHE;
                break;
            case SYNC_POSITION:
                bufindex = 0;
                ReportData->Button = 0;
                ReportData->LX = STICK_CENTER;
                ReportData->LY = STICK_CENTER;
                ReportData->RX = STICK_CENTER;
                ReportData->RY = STICK_CENTER;
                ReportData->HAT = HAT_CENTER;
                state = BREATHE;
                break;
            case BREATHE:
                state = PROCESS;
                break;
                case PROCESS:
                switch (step[bufindex].button)
                {
                    case TRIGGERS:
                        ReportData->Button |= SWITCH_L | SWITCH_R;
                        break;
                    case READ_INPUT:
                        if(W_bool) { ReportData->LY = STICK_MIN; } // FORWARD
                        if(A_bool) { ReportData->LX = STICK_MIN; } // LEFT
                        if(S_bool) { ReportData->LY = STICK_MAX; } // BACKWARD
                        if(D_bool) { ReportData->LX = STICK_MAX; } // RIGHT:
                        if(J_bool) { ReportData->Button |= SWITCH_ZR; } // FIRE!
                        if(U_bool) { ReportData->Button |= SWITCH_X; } // U (Map, x)
                        if(I_bool) { ReportData->Button |= SWITCH_R; } // I (SUB)
                        if(K_bool) { ReportData->Button |= SWITCH_RCLICK; } // M (SPECIAL)
                        if(R_bool) { ReportData->Button |= SWITCH_Y; } // R (Reset camera,y)
                        if(M_bool) { ReportData->Button |= SWITCH_A; } // CONFIRM (A)
                        if(Q_bool) { ReportData->RX = STICK_MIN; } // LOOK LEFT
                        if(E_bool) { ReportData->RX = STICK_MAX; } // LOOK RIGHT
                        if(_SQUID_OR_WALK_) { ReportData->Button |= SWITCH_ZL; } // SQUID OR WALK: (F)

                        if(SPACE_bool) // JUMP!
                        {
                            ReportData->Button |= SWITCH_B;
                            SPACE_bool = false;
                        }

                        // [Mouse Roadmap]:
                        // 1 | -300x = left-side of outer-ring    (255 = MAX_STICK)
                        // 2 | -35x  = left-side of center-ring   (MIN_STICK starting here?)
                        // 3 | 0x (center stick)
                        // 4 | +35x = right-side of center-ring
                        // 5 | +300x = right-side of outer-ring
                        // 6 | -300y = left-side of outer-ring
                        // 7 | -35y  = left-side of center-ring
                        // 8 | 0y (center stick)
                        // 9 | +35y = right-side of center-ring
                        // 0 | +300y = right-side of outer-ring
                        //-------------------------------------

                        // [RX]:
                        if(MOUSE_SPEED==1)
                        {
                            ReportData->RX = STICK_MIN; 
                        } else if(MOUSE_SPEED==2) {
                            ReportData->RX = 64;
                        } else if(MOUSE_SPEED==3) {
                            ReportData->RX = STICK_CENTER;
                        } else if(MOUSE_SPEED==4) {
                            ReportData->RX = 192;
                        } else if(MOUSE_SPEED==5) {
                            ReportData->RX = STICK_MAX;
                        }

                        // [RY]:
                        if(MOUSE_SPEED==6)
                        {
                            ReportData->RY = STICK_MIN; 
                        } else if(MOUSE_SPEED==7) {
                            ReportData->RY = 64;
                        } else if(MOUSE_SPEED==8) {
                            ReportData->RY = STICK_CENTER;
                        } else if(MOUSE_SPEED==9) {
                            ReportData->RY = 192;
                        } else if(MOUSE_SPEED==0) {
                            ReportData->RY = STICK_MAX;
                        }
                        // HATS for left buttons (BOOYA, etc)
                        //ReportData->HAT = HAT_TOP
                        //ReportData->HAT = HAT_BOTTOM
                        break;
                    default:
                        ReportData->LX = STICK_CENTER;
                        ReportData->LY = STICK_CENTER;
                        ReportData->RX = STICK_CENTER;
                        ReportData->RY = STICK_CENTER;
                        ReportData->HAT = HAT_CENTER;
                        break;
                }

                duration_count++;
                if (duration_count > step[bufindex].duration)
                {
                    bufindex++;
                    duration_count = 0;
                }

                if (bufindex > (int)( sizeof(step) / sizeof(step[0])) - 1)
                {
                    bufindex = 7;
                    duration_count = 0;
                    state = BREATHE;
                    ReportData->LX = STICK_CENTER;
                    ReportData->LY = STICK_CENTER;
                    ReportData->RX = STICK_CENTER;
                    ReportData->RY = STICK_CENTER;
                    ReportData->HAT = HAT_CENTER;
                }
                break;
            case CLEANUP:
                return;
        }
    }

    // Prepare to echo this report
    memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
    echoes = ECHOES;
}