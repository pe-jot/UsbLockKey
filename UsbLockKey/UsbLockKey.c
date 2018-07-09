/*
 * UsbTestKeyboard.c
 *
 * Created: 22.01.2015 11:48:36
 *  Author: janosp
 */ 


#include "UsbLockKey.h"


/* Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

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
					.Size                 = KEYBOARD_EPSIZE,
					.Banks                = 1,
				},
			.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
			.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
		},
};


/* Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void) { }


/* Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void) { }


/* Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	USB_Device_EnableSOFEvents();
}


/* Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}


/* Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
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
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

	if (DI2)
	{
		KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTGUI;
		KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_R;
	}
	else if (DI3)
	{
		// Maybe 0xA8 (System Hibernate) instead of 0x82 ??
		#define HID_KEYBOARD_SC_SYSTEM_SLEEP 0x82
		KeyboardReport->KeyCode[5] = HID_KEYBOARD_SC_SYSTEM_SLEEP;
	}
	else if (DI4)
	{
		KeyboardReport->KeyCode[3] = 0x00;
		KeyboardReport->KeyCode[4] = 0xE2;
	}
	else
	{
		KeyboardReport->KeyCode[0] = HID_KEY_NONE;
		KeyboardReport->KeyCode[3] = HID_KEY_NONE;
		KeyboardReport->KeyCode[4] = HID_KEY_NONE;
		KeyboardReport->KeyCode[5] = HID_KEY_NONE;
	}

	*ReportID = 0;
	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	
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


#define MAGIC_BOOT_KEY	0xDEADBEEF
uint32_t Boot_Key ATTR_NO_INIT;	// Mark this variable to be uninitialized when booting

void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void Bootloader_Jump_Check(void)
{
	// If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
	if ((MCUSR & (1 << WDRF)) && (Boot_Key == MAGIC_BOOT_KEY))
	{
		Boot_Key = 0;
		// Bootloader section starts at address 0x3800 (as configured in fuses)
		// BOOTRST fuse is unprogrammed to directly start into application
		asm volatile ("jmp 0x3800");
	}
}


/***********************************************************************************************************************************************************************/


ISR(TIMER1_OVF_vect)
{
	static unsigned char ucButtonHistory = 0;
	
	TCNT1 = TIMER_PRELOAD_200MS;
	char tempSREG = SREG;
	
	ucButtonHistory <<= 1;
	ucButtonHistory |= DI5;
	
	if ((ucButtonHistory & 0b00111111) == 0b00111100)
	{
		LED_RX_TOGGLE;
		ucButtonHistory = 0;
	}
	
	SREG = tempSREG;
}


/***********************************************************************************************************************************************************************/


int main(void)
{	
	// Configure I/O Ports
    DDRB |= (1<<0);
    PORTB = (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7);
    DDRC |= (1<<7);
    PORTC = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6);
    DDRD |= (1<<5) | (1<<6);
    PORTD = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<7);
	
	// Configure Timer 1
	//TCCR1A = 0x00; // Default value
	TCCR1B = 0x05;	// fosc / 1024
	//TCCR1C = 0x00; // Default value
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
		USB_USBTask();
		
		// Back to bootloader
		if (DI11)
		{
			LED_ON;
			
			// If USB is used, detach from the bus and reset it
			USB_Disable();
			
			// Disable all interrupts
			cli();
			
			// Wait two seconds for the USB detachment to register on the host
			_delay_ms(2000);
					
			// Set the bootloader key to the magic value and force a reset through watchdog
			Boot_Key = MAGIC_BOOT_KEY;
			wdt_enable(WDTO_15MS);
			while(1);
		}
    }
}
