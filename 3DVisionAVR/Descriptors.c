/*
             LUFA Library
     Copyright (C) Dean Camera, 2018.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  USB Device Descriptors, for library use when in USB device mode. Descriptors are special
 *  computer-readable structures which the host requests upon device enumeration, to determine
 *  the device's capabilities and functions.
 */

#include "Descriptors.h"


// Descriptor strings
// TODO: obtain these during firmware upload
const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"Copyright (c) 2010 NVIDIA Corporation");
const USB_Descriptor_String_t PROGMEM ProductString = USB_STRING_DESCRIPTOR(L"NVIDIA stereo controller");
const USB_Descriptor_String_t PROGMEM ThirdString = USB_STRING_DESCRIPTOR(L"NVIDIA professional stereo controller");
const USB_StdDescriptor_String_t PROGMEM VersionString = {
	.bLength = sizeof(USB_Descriptor_Header_t) + (10 - 2),
	.bDescriptorType = DTYPE_String,
    .bString = {0x6fbc, 0xd628, 0x9043, 0x29d7}, // 6.14.13.5265 2015-07-13 
	//.bString = {0x4671, 0x20B6, 0xB98E, 0xDF49}, // 6.14.11.9562 2009-11-20 
    //.bString = {0x10be, 0x42b1, 0xef41, 0xbd4e}, // 6.14.11.9062 2009-08-17 (190.62)
};


/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
	.USBSpecification       = VERSION_BCD(2,0,0),
	.Class                  = 0x00, //USB_CSCP_NoDeviceClass
	.SubClass               = 0x00, //USB_CSCP_NoDeviceSubclass
	.Protocol               = 0x00, //USB_CSCP_NoDeviceProtocol

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
	.VendorID               = 0x0955,
	.ProductID              = 0x0007,
	.ReleaseNumber          = VERSION_BCD(3,0,0),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = NO_DESCRIPTOR,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
	.Config =
	{
		.bLength                = sizeof(USB_StdDescriptor_Configuration_Header_t),
		.bDescriptorType        = DTYPE_Configuration,
		.wTotalLength           = sizeof(USB_Descriptor_Configuration_t),
		.bNumInterfaces         = 1,
		.bConfigurationValue    = 1,
		.iConfiguration         = NO_DESCRIPTOR,
		.bmAttributes           = 0x80,
		.bMaxPower              = USB_CONFIG_POWER_MA(400),
	},
	.Emitter_Interface =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
		.InterfaceNumber        = INTERFACE_ID_Emitter,
		.AlternateSetting       = 0,
		.TotalEndpoints         = 4,
		.Class                  = 0xFF,
		.SubClass               = 0x00,
		.Protocol               = 0x00,
		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.Emitter_TransmitEp_Out =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = EMITTER_EP_SWAP_OUT,
		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = EMITTER_EPSIZE,
		.PollingIntervalMS      = 0x01
	},
	.Emitter_TransmitEp_In =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = EMITTER_EP_BUTTON_IN,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = EMITTER_EPSIZE,
		.PollingIntervalMS      = 0x01
	},
	.Emitter_ControlEp_Out =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = EMITTER_EP_CONTROL_OUT,
		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = EMITTER_EPSIZE,
		.PollingIntervalMS      = 0x00
	},
	.Emitter_ControlEp_In =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = EMITTER_EP_CONTROL_IN,
		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = EMITTER_EPSIZE,
		.PollingIntervalMS      = 0x01
	},
};


/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const DescriptorAddress)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case STRING_ID_Language:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case STRING_ID_Manufacturer:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case STRING_ID_Product:
					Address = &ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
				case STRING_ID_Version:
					Address = &VersionString;
					Size    = pgm_read_byte(&VersionString.bLength);
					break;
				case STRING_ID_3:
				default:
					Address = &ThirdString;
					Size    = pgm_read_byte(&ThirdString.Header.Size);
					break;
			}
			break;
	}
	*DescriptorAddress = Address;
	return Size;
}

