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
    // Need to be able to send combo commands like:
    //{ [LSTICK_UP,SHOOT]  200 },
    //Instead of SWIM_UP use?:
    //{ [SQUID ,LSTICK_UP]  200 },
 
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
    /*
    //{ RSTICK_LEFT,   10 }, // Turn camera (about) 90 degrees LEFT
    //{ UP,        30 }, // Move forward 30 (about) 1 lines in test area
    //{ UP,        60 }, // Move forward 60 (about) 2 lines in test area
    //{ UP,        90 }, // Move Forward 90 (about) 3 lines in test area
    */
};

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

        // [Serial commu    nication]:
        if (uart_available()) 
        {
            incomingByte = uart_getchar();
            uart_putchar(incomingByte); // ACK photon
            //if (incomingByte=='\n')
            //{
            //    uart_putchar('7');
            //}
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
int _SQUID_OR_WALK = 0;

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
                    // [w]: if(incomingByte==119)
                    // [W]: if(incomingByte==87)
                    // W // FORWARD: ReportData->LY = STICK_MIN;
                    if(incomingByte==87)
                    {
                        //uart_print_P('UP!')
                        ReportData->LY = STICK_MIN;
                    } else if(incomingByte==119) {
                        ReportData->LY = STICK_CENTER;
                    }

                    // [a]: if(incomingByte==97)
                    // [A]: if(incomingByte==65)
                    // A // LEFT: ReportData->LX = STICK_MIN;
                    if(incomingByte==65)
                    {
                        //uart_print_P('LEFT!')
                        ReportData->LX = STICK_MIN;
                    } else if(incomingByte==97) {
                        ReportData->LX = STICK_CENTER;
                    }

                    // [s]: if(incomingByte==115)
                    // [S]: if(incomingByte==83)
                    // S // BACK: ReportData->LY = STICK_MAX;
                    if(incomingByte==83)
                    {
                        //uart_print_P('BACK!')
                        ReportData->LY = STICK_MAX;
                    } else if(incomingByte==115) {
                        ReportData->LY = STICK_CENTER;
                    }

                    // [d]: if(incomingByte==100)
                    // [D]: if(incomingByte==68)
                    // D // RIGHT: ReportData->LX = STICK_MAX;
                    if(incomingByte==68)
                    {
                        //uart_print_P('RIGHT!')
                        ReportData->LX = STICK_MAX;
                    } else if(incomingByte==100) {
                        ReportData->LX = STICK_CENTER;
                    }

                    // W // FORWARD: ReportData->LY = STICK_MIN;
                    // A // LEFT: ReportData->LX = STICK_MIN;
                    // S // BACK: ReportData->LY = STICK_MAX;
                    // D // RIGHT: ReportData->LX = STICK_MAX;
                    // F // Squid/Walk: //Toggle squid/walk: _SQUID_OR_WALK = ~_SQUID_OR_WALK;_delay_ms(250); if (_SQUID_OR_WALK): ReportData->Button |= SWITCH_ZL;
                    // J // Fire/No Fire: ReportData->Button |= SWITCH_ZR;
                    // R // Confirm (A) ReportData->Button |= SWITCH_A;
                    // SPACE // Jump (B): ReportData->Button |= SWITCH_B;
                    // I // X (Map): //ReportData->Button |= SWITCH_X;
                    // K // A: ReportData->RY = STICK_MAX;
                    // Q left: ReportData->RX = STICK_MIN; ELSE IF: // E right: ReportData->RX = STICK_MAX; ELSE: ReportData->RX = STICK_CENTER;
                    // U (sub): ReportData->Button |= SWITCH_R;
                    // M (special): ReportData->Button |= SWITCH_RCLICK;
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

            ///*
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
            //*/
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