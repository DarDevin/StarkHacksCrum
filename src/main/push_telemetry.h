#ifndef PUSH_TELEMETRY_H
#define PUSH_TELEMETRY_H

#include <Arduino.h>

// Structure to hold the backend response
struct TelemetryResponse {
    bool success;
    bool startSalting;
};

// Send telemetry to FastAPI backend and return the salting decision
TelemetryResponse pushTelemetryToFastAPI(
    const char* serverUrl,
    float latitude,
    float longitude,
    float angleDelta,
    bool isSalting,
    bool isFirstPing
);

#endif // PUSH_TELEMETRY_H