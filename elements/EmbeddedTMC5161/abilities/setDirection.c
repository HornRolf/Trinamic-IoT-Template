ATMO_BOOL_t direction;

ATMO_GetBool(in, &direction);

ATMO_TMC5161_SetDirection(ATMO_VARIABLE(undefined, handle), direction);

return ATMO_Status_Success;
