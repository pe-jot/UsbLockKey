/*
 * UsbLockKey.h
 *
 * Created: 09.07.2018 19:43:06
 *  Author: Philipp
 */ 


#ifndef USBLOCKKEY_H_
#define USBLOCKKEY_H_

	#include <avr/io.h>
	#include <avr/wdt.h>
	#include <avr/power.h>
	#include <avr/interrupt.h>
	#include <util/delay.h>
	#include <stdbool.h>
	#include <stdio.h>

	#include "Descriptors.h"

	#include <LUFA/Drivers/USB/USB.h>
	#include <LUFA/Platform/Platform.h>
	

	#define TIMER_PRELOAD_100MS		0xF9E6
	

	// Onboard LEDs seem to be low-side driven
	#define LED_RX_ON		PORTB &= ~(1 << PB0)
	#define LED_RX_OFF		PORTB |=  (1 << PB0)
	#define LED_TX_ON		PORTD &= ~(1 << PD5)
	#define LED_TX_OFF		PORTD |=  (1 << PD5)
	
	#define LED_1_ON		PORTC |=  (1 << PC6)
	#define LED_1_OFF		PORTC &= ~(1 << PC6)
	#define LED_2_ON		PORTD |=  (1 << PD4)
	#define LED_2_OFF		PORTD &= ~(1 << PD4)

	// Inputs have internal pull-up resistor enabled
	#define S_EMGCY			((PIND & (1 << PD7)) != 0)	// Lock - switch is normally closed
	#define S1				((PIND & (1 << PD0)) == 0)	// Sleep
	#define S2				((PIND & (1 << PD1)) == 0)	// Pause / Next
	
	typedef struct 
	{
		uint8_t Lock;
		uint8_t Pause;
		uint8_t Next;
		uint8_t Sleep;
	} Command_t;
	

	void EVENT_USB_Device_Connect (void);
	void EVENT_USB_Device_Disconnect (void);
	void EVENT_USB_Device_ConfigurationChanged (void);
	void EVENT_USB_Device_ControlRequest (void);
	void EVENT_USB_Device_StartOfFrame (void);

	bool CALLBACK_HID_Device_CreateHIDReport (USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, uint8_t* const ReportID, const uint8_t ReportType, void* ReportData, uint16_t* const ReportSize);
	void CALLBACK_HID_Device_ProcessHIDReport (USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, const uint8_t ReportID, const uint8_t ReportType, const void* ReportData, const uint16_t ReportSize);

#endif /* USBLOCKKEY_H_ */