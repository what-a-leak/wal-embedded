#include "json.h"

// From 'test.json' -> size cannot excess 92 bytes for now
static char _json[128];

char *json_get(payload_t p)
{
    sprintf(_json, "{\"node_id\": %d,\"mesure\": %d,\"timestamp\": %ld,\"batterie\": %d,\"temperature\": %d}", p.node_id, p.reduced_fft[0], p.timestamp, p.battery_level,
            p.temperature);
    return _json;
}

