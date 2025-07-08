#ifndef AWG_H
#define AWG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include <scpi/scpi.h>



typedef enum waveform_type_t
{
    WAVEFORM_TYPE_SINE,
    WAVEFORM_TYPE_SQUARE,
    WAVEFORM_TYPE_TRIANGLE,
    WAVEFORM_TYPE_RAMP,
} waveform_type_t;

typedef struct Channel
{
    double sample_rate;
    double sample_size;
    double sample_format;
    int sample_count;
    double frequency;
    double amplitude;
    double offset;
    double phase;
    double waveform;
    waveform_type_t waveform_type;
    double waveform_frequency;
} Channel;

struct Awg
{

    double sample_size;
    double sample_format;
    double sample_rate;
    int32_t sample_count;
    int64_t operation_duration;
    scpi_bool_t feature_enabled;
    char device_name;
    double frequency_max;
    double frequency_min;
    double frequency_default;
    double amplitude;
    double offset;
    double phase;
    double waveform;
    waveform_type_t waveform_type;
    double waveform_frequency;

    struct Channel channel[8];
    int channel_count;
};





#endif