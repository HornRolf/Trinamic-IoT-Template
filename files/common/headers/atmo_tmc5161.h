#ifndef __ATMO_TMC5161_H_
#define __ATMO_TMC5161_H_

/******************************************************************************/
/*                                Includes                                    */
/******************************************************************************/
#include "../app_src/atmosphere_platform.h"
#include "../include/spi_basic.h"

#include "TMC5161.h"

/******************************************************************************/
/*                                 Defines                                    */
/******************************************************************************/


/******************************************************************************/
/*                       Structs, Typedefs, and Enums                         */
/******************************************************************************/
typedef struct
{
	ATMO_DriverInstanceHandle_t gpioDriverInstance;
	uint32_t cs_pin;
	uint32_t vmax;
	int8_t dir;
} ATMO_TMC5161_Config_t;


typedef enum
{
	ATMO_TMC5161_Status_Success = 0,
	ATMO_TMC5161_Status_Fail,
} ATMO_TMC5161_Status_t;


/******************************************************************************/
/*                       Public Function Declarations                         */
/******************************************************************************/
void tmc5161_readWriteArray(uint8_t channel, uint8_t *data, size_t length);
ATMO_TMC5161_Status_t ATMO_TMC5161_Init(ATMO_DriverInstanceHandle_t *handle, ATMO_TMC5161_Config_t *config);
ATMO_TMC5161_Status_t ATMO_TMC5161_SetVelocity(ATMO_DriverInstanceHandle_t handle, uint8_t velocity);
ATMO_TMC5161_Status_t ATMO_TMC5161_SetAcceleration(ATMO_DriverInstanceHandle_t handle, uint8_t acceleration);
ATMO_TMC5161_Status_t ATMO_TMC5161_MoveTo(ATMO_DriverInstanceHandle_t handle, uint8_t position);
ATMO_TMC5161_Status_t ATMO_TMC5161_SetDirection(ATMO_DriverInstanceHandle_t handle, ATMO_BOOL_t direction);
ATMO_TMC5161_Status_t ATMO_TMC5161_Stop(ATMO_DriverInstanceHandle_t handle);

#endif
