#ifndef AWG_H
#define AWG_H

#include <stdio.h>
#include <stdlib.h>
#include "scpi/scpi.h"


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

scpi_result_t SCPI_SystemHelpHeaders(scpi_t *context);

scpi_result_t SCPI_MyDevVoltageQuery(scpi_t *context);
scpi_result_t SCPI_MyDevVoltageSet(scpi_t *context);

scpi_result_t SCPI_SampleCountQuery(scpi_t *context);
scpi_result_t SCPI_SampleCountSet(scpi_t *context);

scpi_result_t SCPI_DurationSet(scpi_t *context);
scpi_result_t SCPI_DurationQuery(scpi_t *context);

scpi_result_t SCPI_EnableSet(scpi_t *context);
scpi_result_t SCPI_EnableQuery(scpi_t *context);

scpi_result_t SCPI_WaveFormSet(scpi_t *context);
scpi_result_t SCPI_WaveFormQuery(scpi_t *context);

scpi_result_t SCPI_DevNameSet(scpi_t *context);
scpi_result_t SCPI_DevNameQuery(scpi_t *context);

scpi_result_t SCPI_LoadArbitraryWaveform(scpi_t *context);

scpi_result_t SCPI_FrequencyInstQ(scpi_t *context);

scpi_result_t SCPI_MemoryDataAppend(scpi_t *context);
scpi_result_t SCPI_MemoryDataSet(scpi_t *context);
scpi_result_t SCPI_MemoryDataQuery(scpi_t *context);


// scpi_result_t SCPI_RunCommand(scpi_t *context);
// scpi_result_t SCPI_RunAction(scpi_t *context);


extern scpi_choice_def_t waveform_type[];

#define SCPI_AWG_COMMANDS \
    { .pattern="AWG:VOLTage?",.callback=SCPI_MyDevVoltageQuery }, \
    { .pattern="AWG:VOLTage",.callback=SCPI_MyDevVoltageSet }, \
    { .pattern="AWG:Count",.callback=SCPI_SampleCountSet }, \
    { .pattern="AWG:Count?",.callback=SCPI_SampleCountQuery }, \
    { .pattern="AWG:Duration",.callback=SCPI_DurationSet }, \
    { .pattern="AWG:Duration?",.callback=SCPI_DurationQuery }, \
    { .pattern="AWG:Enable",.callback=SCPI_EnableSet }, \
    { .pattern="AWG:Enable?",.callback=SCPI_EnableQuery }, \
    { .pattern="AWG:WAVEform",.callback=SCPI_WaveFormSet }, \
    { .pattern="AWG:WAVEform?",.callback=SCPI_WaveFormQuery }, \
    { .pattern="AWG:NAME",.callback=SCPI_DevNameSet }, \
    { .pattern="AWG:NAME?",.callback=SCPI_DevNameQuery }, \
    { .pattern="AWG:ARB:LOAD",.callback=SCPI_LoadArbitraryWaveform }, \
    { .pattern="AWG:FREQ:INST?",.callback=SCPI_FrequencyInstQ }, \
    { .pattern="SYSTem:HELP:HEADers?",.callback=SCPI_SystemHelpHeaders }, \
    { .pattern="MMEMory:DATA",.callback=SCPI_MemoryDataSet }, \
    { .pattern="MMEMory:DATA?",.callback=SCPI_MemoryDataQuery }, \
    { .pattern="MMEMory:DATA:APPend",.callback=SCPI_MemoryDataAppend }

#endif