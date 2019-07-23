/******************************************************************************/
/*                                Includes                                    */
/******************************************************************************/
#include "atmo_tmc5161.h"


/******************************************************************************/
/*                                 Defines                                    */
/******************************************************************************/
#define _ATMO_MAX_TMC5161_INSTANCES		2
#define _SPI_CS		P_CS

// Scale factors
#define TMC5161_MAX_VELOCITY					1.5E6
#define TMC5161_MAX_ACCELERATION			2500
#define TMC5161_MAX_POSITION					51200

/******************************************************************************/
/*                       Structs, Typedefs, and Enums                         */
/******************************************************************************/


/******************************************************************************/
/*                            Global Variables                                */
/******************************************************************************/
static uint8_t currNumInstances = 0;
static ATMO_TMC5161_Config_t _configs[_ATMO_MAX_TMC5161_INSTANCES];

// Either 0x21 or 0x31 will work for the CTRLA value
const uint8_t tmc5161_ctrla = 1 << SPI_CLK2X_bp    /* Enable Double Speed: disabled */
| 0 << SPI_DORD_bp   /* Data Order Setting: disabled */
| 1 << SPI_ENABLE_bp /* Enable Module: enabled */
| 1 << SPI_MASTER_bp /* SPI module in master mode */
| SPI_PRESC_DIV4_gc; /* System Clock / 4 */

// Must use SPI Mode 3
const uint8_t tmc5161_ctrlb =  0 << SPI_BUFEN_bp   /* Buffer Mode Enable: disabled */
| 0 << SPI_BUFWR_bp /* Buffer Write Mode: disabled */
| 1 << SPI_SSD_bp   /* Slave Select Disable: disabled */
| SPI_MODE_3_gc;    /* SPI Mode 3 */

static uint8_t orig_ctrla, orig_ctrlb;

/******************************************************************************/
/*                      Private Function Definitions                          */
/******************************************************************************/
static void _cs_config(ATMO_DriverInstanceHandle_t gpioDriverIntance, ATMO_SPI_CS_t csPin )
{
	ATMO_GPIO_Config_t pinConfig;

	// configure the CS pin
	pinConfig.pinMode = ATMO_GPIO_PinMode_Output_PushPull;
	pinConfig.initialState = ATMO_GPIO_PinState_High;
	ATMO_GPIO_SetPinConfiguration(gpioDriverIntance, csPin, &pinConfig );
}

static void _spi_config(void)
{
	/* Call before any SPI transaction to make sure the settings are correct */
	orig_ctrla = SPI0.CTRLA;
	orig_ctrlb = SPI0.CTRLB;

	SPI0.CTRLA = tmc5161_ctrla;
	SPI0.CTRLB = tmc5161_ctrlb;
}

static void _spi_restore(void)
{
	/* Call after any SPI transfer to return the original settings */
	SPI0.CTRLA = orig_ctrla;
	SPI0.CTRLB = orig_ctrlb;
}

static void _spi_exchange(uint8_t channel, uint8_t *data, size_t length)
{
	_spi_config();

	// assert CS
	ATMO_GPIO_SetPinState(0, _configs[channel].cs_pin, ATMO_GPIO_PinState_Low);

	// write command and data
	SPI_0_exchange_block(data, length);

	// deassert CS
	ATMO_GPIO_SetPinState(0, _configs[channel].cs_pin, ATMO_GPIO_PinState_High);

	_spi_restore();
}

static void _tmc5161_config(uint8_t channel)
{
	// Example configuration from data sheet
	tmc5161_writeInt(channel, TMC5161_DRV_CONF, 0x20002);
	tmc5161_writeInt(channel, TMC5161_CHOPCONF, 0x100C3);
	//tmc5161_writeInt(channel, TMC5161_IHOLD_IRUN, 0x61F0A);
	tmc5161_writeInt(channel, TMC5161_IHOLD_IRUN, 10 << TMC5161_IHOLDDELAY_SHIFT | 31 << TMC5161_IRUN_SHIFT | 1 << TMC5161_IHOLD_SHIFT);
	tmc5161_writeInt(channel, TMC5161_TPOWERDOWN, 0x0A);
	tmc5161_writeInt(channel, TMC5161_GCONF, 0x04);
	tmc5161_writeInt(channel, TMC5161_TPWMTHRS, 0x1F4);

	/* Current configuration */
	tmc5161_writeInt(channel, TMC5161_GLOBAL_SCALER, 60);	// Global scalar for max motor current at IRUN (1A rms)

	// Motion parameters
	tmc5161_writeInt(channel, TMC5161_A1, 1000);
	tmc5161_writeInt(channel, TMC5161_V1, 50000);
	tmc5161_writeInt(channel, TMC5161_AMAX, 500);
	tmc5161_writeInt(channel, TMC5161_VMAX, 200000);
	tmc5161_writeInt(channel, TMC5161_DMAX, 700);
	tmc5161_writeInt(channel, TMC5161_D1, 1400);
	tmc5161_writeInt(channel, TMC5161_VSTOP, 10);
	tmc5161_writeInt(channel, TMC5161_RAMPMODE, 0);
}

/******************************************************************************/
/*                       Public Function Definitions                          */
/******************************************************************************/
/* This function needs to be defined for the Trinamic API */
void tmc5161_readWriteArray(uint8_t channel, uint8_t *data, size_t length)
{
  _spi_exchange(channel, data, length);
}

ATMO_TMC5161_Status_t ATMO_TMC5161_Init(ATMO_DriverInstanceHandle_t *handle, ATMO_TMC5161_Config_t *config)
{
	if(currNumInstances >= _ATMO_MAX_TMC5161_INSTANCES || config == NULL)
	{
		return ATMO_TMC5161_Status_Fail;
	}

	*handle = currNumInstances;
	currNumInstances++;

	// Default value for VMAX
	config->vmax = 200000;

	memcpy(&_configs[*handle], config, sizeof(ATMO_TMC5161_Config_t));

  _cs_config(config->gpioDriverInstance, config->cs_pin);

	_tmc5161_config(*handle);

  return ATMO_TMC5161_Status_Success;
}

ATMO_TMC5161_Status_t ATMO_TMC5161_SetVelocity(ATMO_DriverInstanceHandle_t handle, uint8_t velocity)
{
	// Scale the input from IoT Studio
	// Max value is 2^23 - 512
	// uint32_t scaled_velocity = (uint32_t) (((float) velocity / 100.0) * 0x7FFE00);
	uint32_t scaled_velocity = (uint32_t) (((float) velocity / 100.0) * TMC5161_MAX_VELOCITY);
	_configs[handle].vmax = scaled_velocity;
	tmc5161_writeInt(handle, TMC5161_VMAX, scaled_velocity);
	// Set V1 to 1/4 of vmax
	tmc5161_writeInt(handle, TMC5161_V1, scaled_velocity >> 2);

  return ATMO_TMC5161_Status_Success;
}

ATMO_TMC5161_Status_t ATMO_TMC5161_SetAcceleration(ATMO_DriverInstanceHandle_t handle, uint8_t acceleration)
{
	// Scale the input from IoT Studio
	// Max value is 2^16-1
	uint16_t scaled_acceleration = (uint16_t) (((float) acceleration / 100.0) * TMC5161_MAX_ACCELERATION);
	tmc5161_writeInt(handle, TMC5161_AMAX, scaled_acceleration);
	// Set A1 to twice AMAX
	tmc5161_writeInt(handle, TMC5161_A1, scaled_acceleration << 1);

  return ATMO_TMC5161_Status_Success;
}

ATMO_TMC5161_Status_t ATMO_TMC5161_MoveTo(ATMO_DriverInstanceHandle_t handle, uint8_t position)
{
	// Scale the input from IoT Studio
	// Value range can be treated as unsigned 0 -> 2^32-1
	// Scale to be within 1 full rotation of 51200
	uint32_t scaled_position = (uint32_t) (((float) position / 100.0) * TMC5161_MAX_POSITION);
  tmc5161_moveTo(handle, _configs[handle].dir * scaled_position, _configs[handle].vmax);

  return ATMO_TMC5161_Status_Success;
}

ATMO_TMC5161_Status_t ATMO_TMC5161_SetDirection(ATMO_DriverInstanceHandle_t handle, ATMO_BOOL_t direction)
{
	uint32_t mode;

	if(direction)
	{
		mode = 0x1;
		_configs[handle].dir = 1;
	}
	else
	{
		mode = 0x2;
		_configs[handle].dir = -1;
	}

	// Set VMAX to 0 first so the motor doesn't explode
	tmc5161_writeInt(handle, TMC5161_VMAX, 0);

	tmc5161_writeInt(handle, TMC5161_RAMPMODE, mode);

	return ATMO_TMC5161_Status_Success;
}

ATMO_TMC5161_Status_t ATMO_TMC5161_Stop(ATMO_DriverInstanceHandle_t handle)
{
  tmc5161_rotate(handle, 0);

  return ATMO_TMC5161_Status_Success;
}
