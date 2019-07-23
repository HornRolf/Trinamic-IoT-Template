ATMO_TMC5161_Config_t config;

config.gpioDriverInstance = ATMO_PROPERTY(undefined, gpioDriverIntance);
config.cs_pin = ATMO_PROPERTY(undefined, csPin);
ATMO_TMC5161_Init(&ATMO_VARIABLE(undefined, handle), &config);

return ATMO_Status_Success;
