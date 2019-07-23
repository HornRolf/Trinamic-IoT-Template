int velocity;

ATMO_GetInt(in, &velocity);
ATMO_TMC5161_SetVelocity(ATMO_VARIABLE(undefined, handle), (uint8_t) velocity);

return ATMO_Status_Success;
