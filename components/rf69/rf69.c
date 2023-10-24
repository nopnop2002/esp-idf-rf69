// RH_RF69.cpp
//
// Copyright (C) 2011 Mike McCauley
// $Id: RH_RF69.cpp,v 1.30 2017/11/06 00:04:08 mikem Exp $

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"

#include <rf69.h>

#define TAG "RF69"


/// Array of instances connected to interrupts 0 and 1
//static RH_RF69*		_deviceForInterrupt[];

/// Index of next interrupt number to use in _deviceForInterrupt
//static uint8_t		_interruptCount;

/// The configured interrupt pin connected to this instance
//static uint8_t				_interruptPin;

/// The index into _deviceForInterrupt[] for this device (if an interrupt is already allocated)
/// else 0xff
//static uint8_t				_myInterruptIndex;

/// The radio OP mode to use when mode is RHModeIdle
static uint8_t _idleMode; 

/// The reported device type
static uint8_t _deviceType;

/// The selected output power in dBm
static int8_t _power;

/// The message length in _buf
static uint8_t _bufLen;

/// Array of octets of teh last received message or the next to transmit message
static uint8_t _buf[RH_RF69_MAX_MESSAGE_LEN];

/// True when there is a valid message in the Rx buffer
static bool _rxBufValid;

/// Time in millis since the last preamble was received (and the last time the RSSI was measured)
static uint32_t	_lastPreambleTime;

#define RH_BROADCAST_ADDRESS 0xff

/// The current transport operating mode
static RHMode _mode = RHModeInitialising;

/// This node id
static uint8_t _thisAddress = RH_BROADCAST_ADDRESS;
	
/// Whether the transport is in promiscuous mode
static bool	_promiscuous;

/// TO header in the last received mesasge
static uint8_t _rxHeaderTo;

/// FROM header in the last received mesasge
static uint8_t _rxHeaderFrom;

/// ID header in the last received mesasge
static uint8_t _rxHeaderId;

/// FLAGS header in the last received mesasge
static uint8_t _rxHeaderFlags;

/// TO header to send in all messages
static uint8_t _txHeaderTo = RH_BROADCAST_ADDRESS;

/// FROM header to send in all messages
static uint8_t _txHeaderFrom = RH_BROADCAST_ADDRESS;

/// ID header to send in all messages
static uint8_t _txHeaderId = 0;

/// FLAGS header to send in all messages
static uint8_t _txHeaderFlags = 0;

/// The value of the last received RSSI value, in some transport specific units
static int16_t _lastRssi;

/// Count of the number of bad messages (eg bad checksum etc) received
//static uint16_t	_rxBad = 0;

/// Count of the number of successfully transmitted messaged
static uint16_t	_rxGood = 0;

/// Count of the number of bad messages (correct checksum etc) received
static uint16_t	_txGood = 0;
	
/// Channel activity detected
//static bool _cad;

/// Channel activity timeout in ms
//static unsigned int _cad_timeout = 0;




// These are indexed by the values of ModemConfigChoice
// Stored in flash (program) memory to save SRAM
// It is important to keep the modulation index for FSK between 0.5 and 10
// modulation index = 2 * Fdev / BR
// Note that I have not had much success with FSK with Fd > ~5
// You have to construct these by hand, using the data from the RF69 Datasheet :-(
// or use the SX1231 starter kit software (Ctl-Alt-N to use that without a connected radio)
#define CONFIG_FSK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_NONE)
#define CONFIG_GFSK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0)
#define CONFIG_OOK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_OOK | RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE)

// Choices for RH_RF69_REG_37_PACKETCONFIG1:
#define CONFIG_NOWHITE (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_NONE | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
#define CONFIG_WHITE (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_WHITENING | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
#define CONFIG_MANCHESTER (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
static const ModemConfig MODEM_CONFIG_TABLE[] =
{
	//	02,		   03,	 04,   05,	 06,   19,	 1a,  37
	// FSK, No Manchester, no shaping, whitening, CRC, no address filtering
	// AFC BW == RX BW == 2 x bit rate
	// Low modulation indexes of ~ 1 at slow speeds do not seem to work very well. Choose MI of 2.
	{ CONFIG_FSK,  0x3e, 0x80, 0x00, 0x52, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb2Fd5
	{ CONFIG_FSK,  0x34, 0x15, 0x00, 0x4f, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb2_4Fd4_8
	{ CONFIG_FSK,  0x1a, 0x0b, 0x00, 0x9d, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb4_8Fd9_6

	{ CONFIG_FSK,  0x0d, 0x05, 0x01, 0x3b, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb9_6Fd19_2
	{ CONFIG_FSK,  0x06, 0x83, 0x02, 0x75, 0xf3, 0xf3, CONFIG_WHITE}, // FSK_Rb19_2Fd38_4
	{ CONFIG_FSK,  0x03, 0x41, 0x04, 0xea, 0xf2, 0xf2, CONFIG_WHITE}, // FSK_Rb38_4Fd76_8

	{ CONFIG_FSK,  0x02, 0x2c, 0x07, 0xae, 0xe2, 0xe2, CONFIG_WHITE}, // FSK_Rb57_6Fd120
	{ CONFIG_FSK,  0x01, 0x00, 0x08, 0x00, 0xe1, 0xe1, CONFIG_WHITE}, // FSK_Rb125Fd125
	{ CONFIG_FSK,  0x00, 0x80, 0x10, 0x00, 0xe0, 0xe0, CONFIG_WHITE}, // FSK_Rb250Fd250
	{ CONFIG_FSK,  0x02, 0x40, 0x03, 0x33, 0x42, 0x42, CONFIG_WHITE}, // FSK_Rb55555Fd50

	//	02,		   03,	 04,   05,	 06,   19,	 1a,  37
	// GFSK (BT=1.0), No Manchester, whitening, CRC, no address filtering
	// AFC BW == RX BW == 2 x bit rate
	{ CONFIG_GFSK, 0x3e, 0x80, 0x00, 0x52, 0xf4, 0xf5, CONFIG_WHITE}, // GFSK_Rb2Fd5
	{ CONFIG_GFSK, 0x34, 0x15, 0x00, 0x4f, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb2_4Fd4_8
	{ CONFIG_GFSK, 0x1a, 0x0b, 0x00, 0x9d, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb4_8Fd9_6

	{ CONFIG_GFSK, 0x0d, 0x05, 0x01, 0x3b, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb9_6Fd19_2
	{ CONFIG_GFSK, 0x06, 0x83, 0x02, 0x75, 0xf3, 0xf3, CONFIG_WHITE}, // GFSK_Rb19_2Fd38_4
	{ CONFIG_GFSK, 0x03, 0x41, 0x04, 0xea, 0xf2, 0xf2, CONFIG_WHITE}, // GFSK_Rb38_4Fd76_8

	{ CONFIG_GFSK, 0x02, 0x2c, 0x07, 0xae, 0xe2, 0xe2, CONFIG_WHITE}, // GFSK_Rb57_6Fd120
	{ CONFIG_GFSK, 0x01, 0x00, 0x08, 0x00, 0xe1, 0xe1, CONFIG_WHITE}, // GFSK_Rb125Fd125
	{ CONFIG_GFSK, 0x00, 0x80, 0x10, 0x00, 0xe0, 0xe0, CONFIG_WHITE}, // GFSK_Rb250Fd250
	{ CONFIG_GFSK, 0x02, 0x40, 0x03, 0x33, 0x42, 0x42, CONFIG_WHITE}, // GFSK_Rb55555Fd50

	//	02,		   03,	 04,   05,	 06,   19,	 1a,  37
	// OOK, No Manchester, no shaping, whitening, CRC, no address filtering
	// with the help of the SX1231 configuration program
	// AFC BW == RX BW
	// All OOK configs have the default:
	// Threshold Type: Peak
	// Peak Threshold Step: 0.5dB
	// Peak threshiold dec: ONce per chip
	// Fixed threshold: 6dB
	{ CONFIG_OOK,  0x7d, 0x00, 0x00, 0x10, 0x88, 0x88, CONFIG_WHITE}, // OOK_Rb1Bw1
	{ CONFIG_OOK,  0x68, 0x2b, 0x00, 0x10, 0xf1, 0xf1, CONFIG_WHITE}, // OOK_Rb1_2Bw75
	{ CONFIG_OOK,  0x34, 0x15, 0x00, 0x10, 0xf5, 0xf5, CONFIG_WHITE}, // OOK_Rb2_4Bw4_8
	{ CONFIG_OOK,  0x1a, 0x0b, 0x00, 0x10, 0xf4, 0xf4, CONFIG_WHITE}, // OOK_Rb4_8Bw9_6
	{ CONFIG_OOK,  0x0d, 0x05, 0x00, 0x10, 0xf3, 0xf3, CONFIG_WHITE}, // OOK_Rb9_6Bw19_2
	{ CONFIG_OOK,  0x06, 0x83, 0x00, 0x10, 0xf2, 0xf2, CONFIG_WHITE}, // OOK_Rb19_2Bw38_4
	{ CONFIG_OOK,  0x03, 0xe8, 0x00, 0x10, 0xe2, 0xe2, CONFIG_WHITE}, // OOK_Rb32Bw64

//	  { CONFIG_FSK,  0x68, 0x2b, 0x00, 0x52, 0x55, 0x55, CONFIG_WHITE}, // works: Rb1200 Fd 5000 bw10000, DCC 400
//	  { CONFIG_FSK,  0x0c, 0x80, 0x02, 0x8f, 0x52, 0x52, CONFIG_WHITE}, // works 10/40/80
//	  { CONFIG_FSK,  0x0c, 0x80, 0x02, 0x8f, 0x53, 0x53, CONFIG_WHITE}, // works 10/40/40

};


// SPI Stuff
#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#endif
static spi_device_handle_t _handle;

// Arduino compatible macros
#define LOW 0
#define HIGH 1
#define delayMicroseconds(us) esp_rom_delay_us(us)
#define delay(ms) esp_rom_delay_us(ms*1000)
#define millis() xTaskGetTickCount()*portTICK_PERIOD_MS

void spi_init() {
	gpio_reset_pin(CONFIG_NSS_GPIO);
	gpio_set_direction(CONFIG_NSS_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(CONFIG_NSS_GPIO, 1);

	spi_bus_config_t buscfg = {
		.sclk_io_num = CONFIG_SCK_GPIO, // set SPI CLK pin
		.mosi_io_num = CONFIG_MOSI_GPIO, // set SPI MOSI pin
		.miso_io_num = CONFIG_MISO_GPIO, // set SPI MISO pin
		.quadwp_io_num = -1,
		.quadhd_io_num = -1
	};

	esp_err_t ret;
	ret = spi_bus_initialize( HOST_ID, &buscfg, SPI_DMA_CH_AUTO );
	ESP_LOGI(TAG, "spi_bus_initialize=%d",ret);
	assert(ret==ESP_OK);

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 5000000, // SPI clock is 5 MHz!
		.queue_size = 7,
		.mode = 0, // SPI mode 0
		.spics_io_num = -1, // we will use manual CS control
		.flags = SPI_DEVICE_NO_DUMMY
	};

	ret = spi_bus_add_device( HOST_ID, &devcfg, &_handle);
	ESP_LOGI(TAG, "spi_bus_add_device=%d",ret);
	assert(ret==ESP_OK);
}

uint8_t spi_transfer(uint8_t address)
{
	uint8_t datain[1];
	uint8_t dataout[1];
	dataout[0] = address;
	//spi_write_byte(dev, dataout, 1 );
	//spi_read_byte(datain, dataout, 1 );

	spi_transaction_t SPITransaction;
	memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
	SPITransaction.length = 8;
	SPITransaction.tx_buffer = dataout;
	SPITransaction.rx_buffer = datain;
	spi_device_transmit( _handle, &SPITransaction );

	return datain[0];
}

uint8_t spiRead(uint8_t reg)
{
	uint8_t val;
	gpio_set_level(CONFIG_NSS_GPIO, LOW);
	spi_transfer(reg & ~RH_RF69_SPI_WRITE_MASK); // Send the address with the write mask off
	val = spi_transfer(0); // The written value is ignored, reg value is read
	gpio_set_level(CONFIG_NSS_GPIO, HIGH);
	return val;
}

uint8_t spiWrite(uint8_t reg, uint8_t val)
{
	uint8_t status = 0;
	gpio_set_level(CONFIG_NSS_GPIO, LOW);
	status = spi_transfer(reg | RH_RF69_SPI_WRITE_MASK); // Send the address with the write mask on
	spi_transfer(val); // New value follows
	gpio_set_level(CONFIG_NSS_GPIO, HIGH);
	return status;
}

uint8_t spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
{
	uint8_t status = 0;
	gpio_set_level(CONFIG_NSS_GPIO, LOW);
	status = spi_transfer(reg & ~RH_RF69_SPI_WRITE_MASK); // Send the start address with the write mask off
	while (len--) *dest++ = spi_transfer(0);
	gpio_set_level(CONFIG_NSS_GPIO, HIGH);
	return status;
}

uint8_t spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
{
	uint8_t status = 0;
	gpio_set_level(CONFIG_NSS_GPIO, LOW);
	status = spi_transfer(reg | RH_RF69_SPI_WRITE_MASK); // Send the start address with the write mask on
	while (len--) spi_transfer(*src++);
	gpio_set_level(CONFIG_NSS_GPIO, HIGH);
	return status;
}

void setIdleMode(uint8_t idleMode)
{
	_idleMode = idleMode;
}

bool init()
{

	// manual reset
	ESP_LOGW(TAG, "Pull high for 100 milliseconds to reset");
	gpio_set_direction(CONFIG_RST_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(CONFIG_RST_GPIO, HIGH);
	delay(100);
	gpio_set_level(CONFIG_RST_GPIO, LOW);
	delay(100);

	_idleMode = RH_RF69_OPMODE_MODE_STDBY;

	spi_init();

	// Get the device type and check it
	// This also tests whether we are really connected to a device
	// My test devices return 0x24
	_deviceType = spiRead(RH_RF69_REG_10_VERSION);
	ESP_LOGI(TAG, "_deviceType=%x", _deviceType);
	if (_deviceType != 0x24) return false;
#if 0
	if (_deviceType == 00 ||
	_deviceType == 0xff)
	return false;
#endif

	setModeIdle();

	// Configure important RH_RF69 registers
	// Here we set up the standard packet format for use by the RH_RF69 library:
	// 4 bytes preamble
	// 2 SYNC words 2d, d4
	// 2 CRC CCITT octets computed on the header, length and data (this in the modem config data)
	// 0 to 60 bytes data
	// RSSI Threshold -114dBm
	// We dont use the RH_RF69s address filtering: instead we prepend our own headers to the beginning
	// of the RH_RF69 payload
	spiWrite(RH_RF69_REG_3C_FIFOTHRESH, RH_RF69_FIFOTHRESH_TXSTARTCONDITION_NOTEMPTY | 0x0f); // thresh 15 is default
	// RSSITHRESH is default
//	  spiWrite(RH_RF69_REG_29_RSSITHRESH, 220); // -110 dbM
	// SYNCCONFIG is default. SyncSize is set later by setSyncWords()
//	  spiWrite(RH_RF69_REG_2E_SYNCCONFIG, RH_RF69_SYNCCONFIG_SYNCON); // auto, tolerance 0
	// PAYLOADLENGTH is default
//	  spiWrite(RH_RF69_REG_38_PAYLOADLENGTH, RH_RF69_FIFO_SIZE); // max size only for RX
	// PACKETCONFIG 2 is default
	spiWrite(RH_RF69_REG_6F_TESTDAGC, RH_RF69_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAOFF);
	// If high power boost set previously, disable it
	spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_NORMAL);
	spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_NORMAL);

	// The following can be changed later by the user if necessary.
	// Set up default configuration
	uint8_t syncwords[] = { 0x2d, 0xd4 };
	setSyncWords(syncwords, sizeof(syncwords)); // Same as RF22's
	// Reasonably fast and reliable default speed and modulation
	setModemConfig(GFSK_Rb250Fd250);

	// 3 would be sufficient, but this is the same as RF22's
	setPreambleLength(4);
	// An innocuous ISM frequency, same as RF22's
	setFrequency(434.0);
	// No encryption
	setEncryptionKey(NULL);
	// +13dBm, same as power-on default
	setTxPower(13, RH_RF69_DEFAULT_HIGHPOWER);

	return true;
}

// Low level function reads the FIFO and checks the address
// Caution: since we put our headers in what the RH_RF69 considers to be the payload, if encryption is enabled
// we have to suffer the cost of decryption before we can determine whether the address is acceptable.
// Performance issue?
void readFifo()
{
	gpio_set_level(CONFIG_NSS_GPIO, LOW);
	spi_transfer(RH_RF69_REG_00_FIFO); // Send the start address with the write mask off
	uint8_t payloadlen = spi_transfer(0); // First byte is payload len (counting the headers)
	if (payloadlen <= RH_RF69_MAX_ENCRYPTABLE_PAYLOAD_LEN &&
	payloadlen >= RH_RF69_HEADER_LEN)
	{
	_rxHeaderTo = spi_transfer(0);
	// Check addressing
	if (_promiscuous ||
		_rxHeaderTo == _thisAddress ||
		_rxHeaderTo == RH_BROADCAST_ADDRESS)
	{
		// Get the rest of the headers
		_rxHeaderFrom  = spi_transfer(0);
		_rxHeaderId    = spi_transfer(0);
		_rxHeaderFlags = spi_transfer(0);
		// And now the real payload
		for (_bufLen = 0; _bufLen < (payloadlen - RH_RF69_HEADER_LEN); _bufLen++)
		_buf[_bufLen] = spi_transfer(0);
		_rxGood++;
		_rxBufValid = true;
	}
	}
	gpio_set_level(CONFIG_NSS_GPIO, HIGH);
	// Any junk remaining in the FIFO will be cleared next time we go to receive mode.
}

int8_t temperatureRead()
{
	// Caution: must be ins standby.
//	  setModeIdle();
	spiWrite(RH_RF69_REG_4E_TEMP1, RH_RF69_TEMP1_TEMPMEASSTART); // Start the measurement
	while (spiRead(RH_RF69_REG_4E_TEMP1) & RH_RF69_TEMP1_TEMPMEASRUNNING)
	; // Wait for the measurement to complete
	return 166 - spiRead(RH_RF69_REG_4F_TEMP2); // Very approximate, based on observation
}

bool setFrequency(float centre)
{
	// Frf = FRF / FSTEP
	uint32_t frf = (uint32_t)((centre * 1000000.0) / RH_RF69_FSTEP);
	spiWrite(RH_RF69_REG_07_FRFMSB, (frf >> 16) & 0xff);
	spiWrite(RH_RF69_REG_08_FRFMID, (frf >> 8) & 0xff);
	spiWrite(RH_RF69_REG_09_FRFLSB, frf & 0xff);

	// afcPullInRange is not used
	//(void)afcPullInRange;
	return true;
}

int8_t rssiRead()
{
	// Force a new value to be measured
	// Hmmm, this hangs forever!
#if 0
	spiWrite(RH_RF69_REG_23_RSSICONFIG, RH_RF69_RSSICONFIG_RSSISTART);
	while (!(spiRead(RH_RF69_REG_23_RSSICONFIG) & RH_RF69_RSSICONFIG_RSSIDONE))
	;
#endif
	return -((int8_t)(spiRead(RH_RF69_REG_24_RSSIVALUE) >> 1));
}

void setOpMode(uint8_t mode)
{
	uint8_t opmode = spiRead(RH_RF69_REG_01_OPMODE);
	opmode &= ~RH_RF69_OPMODE_MODE;
	opmode |= (mode & RH_RF69_OPMODE_MODE);
	spiWrite(RH_RF69_REG_01_OPMODE, opmode);
	ESP_LOGD(TAG, "setOpMode=%x", opmode);

	// Wait for mode to change.
	while (!(spiRead(RH_RF69_REG_27_IRQFLAGS1) & RH_RF69_IRQFLAGS1_MODEREADY))
	;

	// Verify new mode
	uint8_t _opmode = spiRead(RH_RF69_REG_01_OPMODE);
	if (opmode != _opmode) {
		ESP_LOGE(TAG, "setOpMode fail. %x %x", opmode, _opmode);
	}
}

void setModeIdle()
{
	if (_mode != RHModeIdle)
	{
	if (_power >= 18)
	{
		// If high power boost, return power amp to receive mode
		spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_NORMAL);
		spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_NORMAL);
	}
	setOpMode(_idleMode);
	_mode = RHModeIdle;
	}
}

bool setSleep()
{
	if (_mode != RHModeSleep)
	{
	spiWrite(RH_RF69_REG_01_OPMODE, RH_RF69_OPMODE_MODE_SLEEP);
	_mode = RHModeSleep;
	}
	return true;
}

void setModeRx()
{
	if (_mode != RHModeRx)
	{
	if (_power >= 18)
	{
		// If high power boost, return power amp to receive mode
		spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_NORMAL);
		spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_NORMAL);
	}
	spiWrite(RH_RF69_REG_25_DIOMAPPING1, RH_RF69_DIOMAPPING1_DIO0MAPPING_01); // Set interrupt line 0 PayloadReady
	setOpMode(RH_RF69_OPMODE_MODE_RX); // Clears FIFO
	_mode = RHModeRx;
	}
}

void setModeTx()
{
	if (_mode != RHModeTx)
	{
	if (_power >= 18)
	{
		// Set high power boost mode
		// Note that OCP defaults to ON so no need to change that.
		spiWrite(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_BOOST);
		spiWrite(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_BOOST);
	}
	spiWrite(RH_RF69_REG_25_DIOMAPPING1, RH_RF69_DIOMAPPING1_DIO0MAPPING_00); // Set interrupt line 0 PacketSent
	setOpMode(RH_RF69_OPMODE_MODE_TX); // Clears FIFO
	_mode = RHModeTx;
	}
}

void setTxPower(int8_t power, bool ishighpowermodule)
{
  _power = power;
  uint8_t palevel;

  if (ishighpowermodule)
  {
	if (_power < -2)
	  _power = -2; //RFM69HW only works down to -2.
	if (_power <= 13)
	{
	  // -2dBm to +13dBm
	  //Need PA1 exclusivelly on RFM69HW
	  palevel = RH_RF69_PALEVEL_PA1ON | ((_power + 18) &
	  RH_RF69_PALEVEL_OUTPUTPOWER);
	}
	else if (_power >= 18)
	{
	  // +18dBm to +20dBm
	  // Need PA1+PA2
	  // Also need PA boost settings change when tx is turned on and off, see setModeTx()
	  palevel = RH_RF69_PALEVEL_PA1ON
	| RH_RF69_PALEVEL_PA2ON
	| ((_power + 11) & RH_RF69_PALEVEL_OUTPUTPOWER);
	}
	else
	{
	  // +14dBm to +17dBm
	  // Need PA1+PA2
	  palevel = RH_RF69_PALEVEL_PA1ON
	| RH_RF69_PALEVEL_PA2ON
	| ((_power + 14) & RH_RF69_PALEVEL_OUTPUTPOWER);
	}
  }
  else
  {
	if (_power < -18) _power = -18;
	if (_power > 13) _power = 13; //limit for RFM69W
	palevel = RH_RF69_PALEVEL_PA0ON
	  | ((_power + 18) & RH_RF69_PALEVEL_OUTPUTPOWER);
  }
  spiWrite(RH_RF69_REG_11_PALEVEL, palevel);
}

// Sets registers from a canned modem configuration structure
void setModemRegisters(const ModemConfig* config)
{
	spiBurstWrite(RH_RF69_REG_02_DATAMODUL,		&config->reg_02, 5);
	spiBurstWrite(RH_RF69_REG_19_RXBW,			&config->reg_19, 2);
	spiWrite(RH_RF69_REG_37_PACKETCONFIG1,		 config->reg_37);

#if 0
	uint8_t dest[8];
	spiBurstRead(RH_RF69_REG_02_DATAMODUL, dest, 5);
	for (int i=0;i<5;i++) {
		ESP_LOGI(TAG, "RH_RF69_REG_02_DATAMODUL[%d]=%x", i, dest[i]);
	}

	spiBurstRead(RH_RF69_REG_19_RXBW, dest, 2);
	for (int i=0;i<2;i++) {
		ESP_LOGI(TAG, "RH_RF69_REG_19_RXBW[%d]=%x", i, dest[i]);
	}
#endif
}

// Set one of the canned FSK Modem configs
// Returns true if its a valid choice
bool setModemConfig(ModemConfigChoice index)
{
	if (index > (signed int)(sizeof(MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
		return false;

	ModemConfig cfg;
	memcpy(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(ModemConfig));
	setModemRegisters(&cfg);

	return true;
}

void setPreambleLength(uint16_t bytes)
{
	spiWrite(RH_RF69_REG_2C_PREAMBLEMSB, bytes >> 8);
	spiWrite(RH_RF69_REG_2D_PREAMBLELSB, bytes & 0xff);
}

void setSyncWords(const uint8_t* syncWords, uint8_t len)
{
	uint8_t syncconfig = spiRead(RH_RF69_REG_2E_SYNCCONFIG);
	if (syncWords && len && len <= 4)
	{
	spiBurstWrite(RH_RF69_REG_2F_SYNCVALUE1, syncWords, len);

#if 0
	uint8_t dest[8];
	spiBurstRead(RH_RF69_REG_2F_SYNCVALUE1, dest, 8);
	for (int i=0;i<8;i++) {
		ESP_LOGI(TAG, "RH_RF69_REG_2F_SYNCVALUE1[%d]=%x", i, dest[i]);
	}
#endif

	syncconfig |= RH_RF69_SYNCCONFIG_SYNCON;
	}
	else
	syncconfig &= ~RH_RF69_SYNCCONFIG_SYNCON;
	syncconfig &= ~RH_RF69_SYNCCONFIG_SYNCSIZE;
	syncconfig |= (len-1) << 3;
	ESP_LOGD(TAG, "syncconfig=%x", syncconfig);
	spiWrite(RH_RF69_REG_2E_SYNCCONFIG, syncconfig);
}

void setEncryptionKey(uint8_t* key)
{
	if (key)
	{
	spiBurstWrite(RH_RF69_REG_3E_AESKEY1, key, 16);
	spiWrite(RH_RF69_REG_3D_PACKETCONFIG2, spiRead(RH_RF69_REG_3D_PACKETCONFIG2) | RH_RF69_PACKETCONFIG2_AESON);
	}
	else
	{
	spiWrite(RH_RF69_REG_3D_PACKETCONFIG2, spiRead(RH_RF69_REG_3D_PACKETCONFIG2) & ~RH_RF69_PACKETCONFIG2_AESON);
	}
}

bool available()
{
	// Get the interrupt cause
	uint8_t irqflags2 = spiRead(RH_RF69_REG_28_IRQFLAGS2);
	ESP_LOGD(TAG, "available irqflags2=%x", irqflags2);
	// Must look for PAYLOADREADY, not CRCOK, since only PAYLOADREADY occurs _after_ AES decryption
	// has been done
	if (irqflags2 & RH_RF69_IRQFLAGS2_PAYLOADREADY) {
		// A complete message has been received with good CRC
		_lastRssi = -((int8_t)(spiRead(RH_RF69_REG_24_RSSIVALUE) >> 1));
		_lastPreambleTime = millis();

		setModeIdle();
		// Save it in our buffer
		readFifo();
		ESP_LOGD(TAG, "PAYLOADREADY");
	}
	setModeRx(); // Make sure we are receiving
	return _rxBufValid;
}

// Blocks until a valid message is received or timeout expires
// Return true if there is a message available
// Works correctly even on millis() rollover
bool waitAvailableTimeout(uint16_t timeout)
{
	unsigned long starttime = millis();
	while ((millis() - starttime) < timeout)
	{
		if (available())
	{
		   return true;
	}
	vTaskDelay(1);
	}
	return false;
}


bool recv(uint8_t* buf, uint8_t* len)
{
	if (!available())
	return false;

	if (buf && len)
	{
	if (*len > _bufLen)
		*len = _bufLen;
	memcpy(buf, _buf, *len);
	}
	_rxBufValid = false; // Got the most recent message
//	  printBuffer("recv:", buf, *len);
	return true;
}

bool send(const uint8_t* data, uint8_t len)
{
	if (len > RH_RF69_MAX_MESSAGE_LEN)
	return false;

#if 0
	waitPacketSent(); // Make sure we dont interrupt an outgoing message
#endif
	setModeIdle(); // Prevent RX while filling the fifo

#if 0
	if (!waitCAD())
	return false;  // Check channel activity
#endif

	ESP_LOGD(TAG, "_txHeaderTo=%d", _txHeaderTo);
	ESP_LOGD(TAG, "_txHeaderFrom=%d", _txHeaderFrom);
	ESP_LOGD(TAG, "_txHeaderId=%d", _txHeaderId);
	ESP_LOGD(TAG, "_txHeaderFlags=%d", _txHeaderFlags);

	gpio_set_level(CONFIG_NSS_GPIO, LOW);
	spi_transfer(RH_RF69_REG_00_FIFO | RH_RF69_SPI_WRITE_MASK); // Send the start address with the write mask on
	spi_transfer(len + RH_RF69_HEADER_LEN); // Include length of headers

	spi_transfer(_txHeaderTo);
	spi_transfer(_txHeaderFrom);
	spi_transfer(_txHeaderId);
	spi_transfer(_txHeaderFlags);
	// Now the payload
	while (len--)
	spi_transfer(*data++);
	gpio_set_level(CONFIG_NSS_GPIO, HIGH);

	setModeTx(); // Start the transmitter
	vTaskDelay(1);
	return true;
}

bool waitPacketSent()
{
	while (1) {
		// Get the interrupt cause
		uint8_t irqflags2 = spiRead(RH_RF69_REG_28_IRQFLAGS2);
		ESP_LOGD(TAG, "waitPacketSent irqflags2=%x", irqflags2);
		if (irqflags2 & RH_RF69_IRQFLAGS2_PACKETSENT) {
			// A transmitter message has been fully sent
			setModeIdle(); // Clears FIFO
			_txGood++;
			ESP_LOGD(TAG, "PACKETSENT");
			break;
		}
		vTaskDelay(1);
	}
	return true;
}


uint8_t maxMessageLength()
{
	return RH_RF69_MAX_MESSAGE_LEN;
}

bool printRegister(uint8_t reg)
{
	printf("%x %x\n", reg, spiRead(reg));
	return true;
}

bool printRegisters()
{
	uint8_t i;
	for (i = 0; i < 0x50; i++)
	printRegister(i);
	// Non-contiguous registers
	printRegister(RH_RF69_REG_58_TESTLNA);
	printRegister(RH_RF69_REG_6F_TESTDAGC);
	printRegister(RH_RF69_REG_71_TESTAFC);

	return true;
}

uint8_t headerTo()
{
	return _rxHeaderTo;
}

uint8_t headerFrom()
{
	return _rxHeaderFrom;
}

uint8_t headerId()
{
	return _rxHeaderId;
}

uint8_t headerFlags()
{
	return _rxHeaderFlags;
}

int16_t lastRssi()
{
	return _lastRssi;
}



