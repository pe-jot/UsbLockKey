/*
 * UsbTestKeyboard.h
 *
 * Created: 22.01.2015 12:16:34
 *  Author: janosp
 */ 


#ifndef USBLOCKKEY_H_
#define USBLOCKKEY_H_

	#define HID_KEY_NONE	0x00
	
	#define LED_RX_OFF		PORTB |= (1 << PB0)
	#define LED_RX_ON		PORTB &= ~(1 << PB0)
	#define LED_TX_OFF		PORTD |= (1 << PD5)
	#define LED_TX_ON		PORTD &= ~(1 << PD5)
	#define LED_ON			PORTC |= (1 << PC7)
	#define LED_OFF			PORTC &= ~(1 << PC7)
	
	#define DI2				(!(PIND & (1 << PD1)))	/* Lock */
	#define DI11			(!(PINB & (1 << PB7)))	/* Bootmode */
	
	#define DI3				(!(PIND & (1 << PD0)))
	#define DI4				(!(PIND & (1 << PD4)))
	#define DI5				(!(PINC & (1 << PC6)))
	#define DI6				(!(PIND & (1 << PD7)))
	#define DI7				(!(PINE & (1 << PE6)))
	#define DI8				(!(PINB & (1 << PB4)))
	#define DI9				(!(PINB & (1 << PB5)))
	#define DI10			(!(PINB & (1 << PB6)))
	
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
	
	
	
	void SetupHardware(void);

	void EVENT_USB_Device_Connect(void);
	void EVENT_USB_Device_Disconnect(void);
	void EVENT_USB_Device_ConfigurationChanged(void);
	void EVENT_USB_Device_ControlRequest(void);
	void EVENT_USB_Device_StartOfFrame(void);

	bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
	uint8_t* const ReportID,
	const uint8_t ReportType,
	void* ReportData,
	uint16_t* const ReportSize);
	void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
	const uint8_t ReportID,
	const uint8_t ReportType,
	const void* ReportData,
	const uint16_t ReportSize);
	
#endif /* USBLOCKKEY_H_ */