#include "json.h"

#include <string.h>

// From 'test.json' -> size cannot excess 92 bytes for now
static char _json[256];

char *json_get(payload_t p)
{
    char temp[5];
    char measure_str[5*22];
    for (int i = 0; i < 22; i++)
    {
        sprintf(temp, "%d%c", p.reduced_fft[i], i == 21 ? 0 : ',');
        strcat(measure_str, temp);
    }

    sprintf(_json, "{\"node_id\": %d,\"mesure\": \"%s\", \"status\": %d,\"timestamp\": %ld,\"batterie\": %d,\"temperature\": %d}"
    , p.node_id
    , measure_str
    , (p.reduced_fft[11] > 140)
    , p.timestamp
    , p.battery_level
    , p.temperature);
    return _json;
}

