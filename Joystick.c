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

#define BAUD_RATE 9600

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

// write a string to the uart
#define uart_print(s) uart_print_P(PSTR(s))
void uart_print_P(const char *str)
{
	char c;
	while (1) {
		c = pgm_read_byte(str++);
		if (!c) break;
		uart_putchar(c);
	}
}


// Teensy++ 2.0 has the LED on pin 6

// [Define some global buttons]:
typedef enum {
    LSTICK_UP,
    LSTICK_DOWN,
    LSTICK_LEFT,
    LSTICK_RIGHT,
    RSTICK_UP,
    RSTICK_DOWN,
    RSTICK_LEFT,
    RSTICK_RIGHT,
    X,
    Y,
    A,
    B,
    L,
    R,
    THROW_SUB,
    NOTHING,
    TRIGGERS,
    SQUID, ZL,
    SHOOT, ZR,
    SWIM_UP, SWIM_DOWN, SWIM_LEFT, SWIM_RIGHT,
    READ_INPUT,
    RESET
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
    //{ READ_INPUT, 200 }, // Read Input from Voltage on C2!
    /*
    { B,              5 }, // Jump!
    { NOTHING,       20 },
    { LSTICK_UP,     30 }, // Move UP 30 (about) 1 line in test area
    { SHOOT,          5 }, // Ink!
    { NOTHING,       20 },
    { RSTICK_LEFT,    9 }, // Turn left
    { LSTICK_UP,     30 }, // Move UP 30 (about) 1 line in test area
    { SHOOT,          5 }, // Ink!
    { NOTHING,       20 },
    { RSTICK_LEFT,    9 }, // Turn left
    { LSTICK_UP,     30 }, // Move UP 30 (about) 1 line in test area
    { SHOOT,          5 }, // Ink!
    { NOTHING,       20 },
    { RSTICK_LEFT,    9 }, // Turn left
    { LSTICK_UP,     30 }, // Move UP 30 (about) 1 line in test area
    { SHOOT,          5 }, // Ink!
    { NOTHING,       20 },
    { SQUID,        200 }, // Hide!
    */
    { READ_INPUT,   200 }
    //{ READ_INPUT,   20 }
    /*
    //{ RSTICK_LEFT,   10 }, // Turn camera (about) 90 degrees LEFT
    //{ UP,        30 }, // Move forward 30 (about) 1 lines in test area
    //{ UP,        60 }, // Move forward 60 (about) 2 lines in test area
    //{ UP,        90 }, // Move Forward 90 (about) 3 lines in test area
    */
};

bool W_bool = false;
bool A_bool = false;
bool S_bool = false;
bool D_bool = false;
bool E_bool = false;
bool F_bool = false; int _SQUID_OR_WALK = 0;
bool J_bool = false;
bool R_bool = false;
bool I_bool = false;
bool K_bool = false;
bool Q_bool = false;
bool U_bool = false;
bool M_bool = false;
bool SPACE_bool = false;
bool BOOYA_bool = false;
int MOUSE_SPEED = 0;
// HATS for left buttons (BOOYA, etc)
void set_bool(char c)
{
    // Set ONs:
    if (c == 'W') { W_bool = true; }
    if (c == 'A') { A_bool = true; }
    if (c == 'S') { S_bool = true; }
    if (c == 'D') { D_bool = true; }
    if (c == 'E') { E_bool = true; }
    if (c == 'F') { _SQUID_OR_WALK = ~_SQUID_OR_WALK; }
    if (c == 'J') { J_bool = true; }
    if (c == 'R') { R_bool = true; }
    if (c == 'I') { I_bool = true; }
    if (c == 'K') { K_bool = true; }
    if (c == 'Q') { Q_bool = true; }
    if (c == 'U') { U_bool = true; }
    if (c == 'M') { M_bool = true; }
    if (c == ' ') { SPACE_bool = true; }

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
    SYNC_CONTROLLER,
    SYNC_POSITION,
    BREATHE,
    PROCESS,
    CLEANUP,
    DONE
} State_t;
State_t state = SYNC_CONTROLLER;

#define ECHOES 2
int echoes = 0;
USB_JoystickReport_Input_t last_report;

int report_count = 0;
int xpos = 0;
int ypos = 0;
int bufindex = 0;
int duration_count = 0;
int portsval = 0;

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

    // States and moves management
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
                case LSTICK_UP:
                    ReportData->LY = STICK_MIN;
                    break;
                case LSTICK_LEFT:
                    ReportData->LX = STICK_MIN;
                    break;
                case LSTICK_DOWN:
                    ReportData->LY = STICK_MAX;
                    break;
                case LSTICK_RIGHT:
                    ReportData->LX = STICK_MAX;
                    break;
                case RSTICK_UP:
                    ReportData->RY = STICK_MIN;
                    break;
                case RSTICK_LEFT:
                    ReportData->RX = STICK_MIN;
                    break;
                case RSTICK_DOWN:
                    ReportData->RY = STICK_MAX;
                    break;
                case RSTICK_RIGHT:
                    ReportData->RX = STICK_MAX;
                    break;
                case A:
                    ReportData->Button |= SWITCH_A;
                    break;
                case B:
                    ReportData->Button |= SWITCH_B;
                    break;
                case R:
                    ReportData->Button |= SWITCH_R;
                    break;
                case Y:
                    ReportData->Button |= SWITCH_Y;
                    break;
                case THROW_SUB: // composite example
                    ReportData->LY = STICK_MIN;
                    ReportData->Button |= SWITCH_R;
                    break;
                case ZL:
                case SQUID:
                    ReportData->Button |= SWITCH_ZL;
                    break;
                case ZR:
                case SHOOT:
                    ReportData->Button |= SWITCH_ZR;
                    break;
                case TRIGGERS:
                    ReportData->Button |= SWITCH_L | SWITCH_R;
                    break;
                case READ_INPUT:
                    if(W_bool) { ReportData->LY = STICK_MIN; } // FORWARD
                    if(A_bool) { ReportData->LX = STICK_MIN; } // LEFT
                    if(S_bool) { ReportData->LY = STICK_MAX; } // BACKWARD
                    if(D_bool) { ReportData->LX = STICK_MAX; } // RIGHT:
                    if(J_bool) { ReportData->Button |= SWITCH_ZR; } // FIRE!
                    if(R_bool) { ReportData->Button |= SWITCH_X; } // R (Map, x)
                    if(I_bool) { ReportData->Button |= SWITCH_R; } // I (SUB)
                    if(K_bool) { ReportData->Button |= SWITCH_RCLICK; } // M (SPECIAL)
                    if(U_bool) { ReportData->Button |= SWITCH_Y; } // U (Reset camera,y)
                    if(M_bool) { ReportData->Button |= SWITCH_A; } // CONFIRM (A)
                    if(Q_bool) { ReportData->RX = STICK_MIN; } // LOOK LEFT
                    if(E_bool) { ReportData->RX = STICK_MAX; } // LOOK RIGHT
                    if(_SQUID_OR_WALK) { ReportData->Button |= SWITCH_ZL; } // SQUID OR WALK: (F)

                    if(SPACE_bool) // JUMP!
                    {
                        ReportData->Button |= SWITCH_B;
                        SPACE_bool = false;
                    }

                    // 1 = -2
                    // 3 = -1
                    // 5 = 0
                    // 7 = +1
                    // 9 = +2
                    if(MOUSE_SPEED==1)
                    {
                        ReportData->RX = STICK_MIN; 
                    } else if(MOUSE_SPEED==3) {
                        ReportData->RX = 64;
                    } else if(MOUSE_SPEED==5) {
                        ReportData->RX = STICK_CENTER;
                    } else if(MOUSE_SPEED==7) {
                        ReportData->RX = 192;
                    } else if(MOUSE_SPEED==9) {
                        ReportData->RX = STICK_MAX;
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
            state = DONE;
            break;

        case DONE:
            portsval = ~portsval;
            PORTD = portsval; // flash LED(s) and sound buzzer if attached
            PORTB = portsval;
            _delay_ms(250);
            return;
    }

    // Prepare to echo this report
    memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
    echoes = ECHOES;
}