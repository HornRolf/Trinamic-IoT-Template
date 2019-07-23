int acceleration;

ATMO_GetInt(in, &acceleration);
ATMO_TMC5161_SetAcceleration(ATMO_VARIABLE(undefined, handle), acceleration);

return ATMO_Status_Success;
