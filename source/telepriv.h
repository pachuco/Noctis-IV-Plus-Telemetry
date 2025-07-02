#ifndef TELEPRIV_HEADER
#define TELEPRIV_HEADER

void telemetry_init();
void telemetry_halt();
void telemetry_out_perceptualPressure(float val);
void telemetry_out_debugBeep();

#endif