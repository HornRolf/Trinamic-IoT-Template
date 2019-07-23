int position;

// Extract arguments from input buffer
ATMO_GetInt(in, &position);
ATMO_TMC5161_MoveTo(ATMO_VARIABLE(undefined, handle), position);

return ATMO_Status_Success;
