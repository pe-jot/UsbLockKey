/*
 * UsbTestKeyboard.c
 *
 * Created: 22.01.2015 11:48:36
 *  Author: janosp
 */ 


#include "UsbLockKey.h"


/* Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];
static uint8_t PrevMediaControlHIDReportBuffer[sizeof(USB_MediaReport_Data_t)];
static Command_t gCommand;


/*  LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
{
	.Config =
	{
		.InterfaceNumber              = INTERFACE_ID_Keyboard,
		.ReportINEndpoint             =
		{
			.Address              = KEYBOARD_EPADDR,
			.Size                 = HID_EPSIZE,
			.Banks                = 1,
		},
		.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
		.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
	},
};

USB_ClassInfo_HID_Device_t MediaControl_HID_Interface =
{
	.Config =
	{
		.InterfaceNumber              = INTERFACE_ID_Media,
		.ReportINEndpoint             =
		{
			.Address              = MEDIACONTROL_HID_EPADDR,
			.Size                 = HID_EPSIZE,
			.Banks                = 1,
		},
		.PrevReportINBuffer           = PrevMediaControlHIDReportBuffer,
		.PrevReportINBufferSize       = sizeof(PrevMediaControlHIDReportBuffer),
	},
};


/* Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void) 
{}


/* Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{}


/* Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&MediaControl_HID_Interface);
	
	USB_Device_EnableSOFEvents();
}


/* Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
	HID_Device_ProcessControlRequest(&MediaControl_HID_Interface);
}


/* Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
	HID_Device_MillisecondElapsed(&MediaControl_HID_Interface);
}


/*  HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	/* Determine which interface must have its report generated */
	if (HIDInterfaceInfo == &Keyboard_HID_Interface)
	{
		USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
		
		/* Windows Lock */
		if (gCommand.Lock)
		{
			gCommand.Lock = 0;
			KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTGUI;
			KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_L;
		}
		else
		{
			KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_NONE;
			KeyboardReport->KeyCode[0] = HID_KEY_NONE;
		}
			
		/* System Sleep */
		#define HID_KEYBOARD_SC_SYSTEM_SLEEP 0x82	/* Maybe 0xA8 (System Hibernate) instead of 0x82 ?? */
		if (gCommand.Sleep)
		{
			gCommand.Sleep = 0;
			KeyboardReport->KeyCode[5] = HID_KEYBOARD_SC_SYSTEM_SLEEP;
		}
		else
		{
			KeyboardReport->KeyCode[5] = HID_KEY_NONE;
		}
			
		*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	}
	else if (HIDInterfaceInfo == &MediaControl_HID_Interface)
	{
		USB_MediaReport_Data_t* MediaReport = (USB_MediaReport_Data_t*)ReportData;
		
		/* Music Stop */
		if (gCommand.Pause)
		{
			gCommand.Pause = 0;
			MediaReport->PlayPause = 1;
		}

		*ReportSize = sizeof(USB_MediaReport_Data_t);
	}

	return false;
}



/*  HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{ }


/***********************************************************************************************************************************************************************/


ISR(TIMER1_OVF_vect)
{
	static unsigned char ucDi2History = 0;
	static unsigned char ucDi3History = 0;
	static unsigned char ucDi4History = 0;
	
	TCNT1 = TIMER_PRELOAD_200MS;
	char tempSREG = SREG;
	
	ucDi2History <<= 1;
	ucDi2History |= DI2;
	ucDi3History <<= 1;
	ucDi3History |= DI3;
	ucDi4History <<= 1;
	ucDi4History |= DI4;
	
	if ((ucDi2History & 0b00000011) == 0b00000010)
	{
		gCommand.Lock = 1;
		ucDi2History = 0;
	}
	
	if ((ucDi3History & 0b00000011) == 0b00000010)
	{
		gCommand.Sleep = 1;
		ucDi3History = 0;
	}
	
	if ((ucDi4History & 0b00000011) == 0b00000010)
	{
		gCommand.Pause = 1;
		ucDi4History = 0;
	}
	
	SREG = tempSREG;
}


/***********************************************************************************************************************************************************************/


int main(void)
{
	memset(&gCommand, 0, sizeof(Command_t));
	
	// Configure I/O Ports
    DDRB |= (1<<0);
    PORTB = (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7);
    DDRC |= (1<<7);
    PORTC = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6);
    DDRD |= (1<<5) | (1<<6);
    PORTD = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<7);
	
	// Start Bootloader if DI11 is pressed during reset OR DI2 is pressed during power on
	if ( ((MCUSR & (1 << EXTRF)) && DI11) || ((MCUSR & (1 << PORF)) && DI2) )
	{
		LED_ON;
		// Bootloader section starts at address 0x3800 (as configured in fuses)
		// BOOTRST fuse is unprogrammed to directly start into application
		asm volatile ("jmp 0x3800");
	}
	
	// Configure Timer 1
	TCCR1B = 0x05;	// fosc / 1024
	TCNT1 = TIMER_PRELOAD_200MS;
	TIMSK1 = (1 << TOIE1);	// Timer1 Overflow Interrupt

	// Disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Disable clock division
	clock_prescale_set(clock_div_1);

	USB_Init();
	GlobalInterruptEnable();
	
    while(1)
    {	
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&MediaControl_HID_Interface);
		USB_USBTask();
    }
}
