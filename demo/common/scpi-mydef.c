#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scpi/scpi.h"
#include "scpi-def.h"
#include "awg.h"

struct Awg awg = {0};

#define MAX_FILE_NAME_LENGTH 256

//Implementation of double type voltage query command
scpi_result_t SCPI_MyDevVoltageQuery(scpi_t *context) {
    SCPI_ResultDouble(context, awg.amplitude);
    return SCPI_RES_OK;
}
//Implementation of double type voltage setting command
scpi_result_t SCPI_MyDevVoltageSet(scpi_t *context) {

    double val = 0;
    if (!SCPI_ParamDouble(context, &val, TRUE)) {
        return SCPI_RES_ERR;
    }

    awg.amplitude = val;
    return SCPI_RES_OK;
}

//Implementation of Counting Query Command现
scpi_result_t SCPI_SampleCountQuery(scpi_t *context){
    SCPI_ResultInt32(context, awg.sample_count);
    return SCPI_RES_OK;
}
//Set Operation Duration Command
scpi_result_t SCPI_SampleCountSet(scpi_t *context){
    int32_t count = 0;
    if (!SCPI_ParamInt32(context, &count, TRUE)) {
        return SCPI_RES_ERR;
    }

    awg.sample_count = count;
    return SCPI_RES_OK;
}


//Set Operation Duration Command
scpi_result_t SCPI_DurationSet(scpi_t *context){
    int64_t duration = 0;
    if (!SCPI_ParamInt64(context, &duration, TRUE)) {
        return SCPI_RES_ERR;
    }

    awg.operation_duration = duration;
    return SCPI_RES_OK;
}
//Query Operation Duration Command
scpi_result_t SCPI_DurationQuery(scpi_t *context){
    SCPI_ResultInt64(context,awg.operation_duration);
    return SCPI_RES_OK;
}

//Implementation of the Enable Function Switch Command
scpi_result_t SCPI_EnableSet(scpi_t *context) {
    scpi_bool_t enable = FALSE; 

     if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR; 
    }

    awg.feature_enabled = enable; 

    return SCPI_RES_OK;
}
//Implementation of query enabling function switch command
scpi_result_t SCPI_EnableQuery(scpi_t *context) {
    SCPI_ResultBool(context, awg.feature_enabled);
    return SCPI_RES_OK;
}


//Definition of waveform type enumeration
scpi_choice_def_t waveform_type[] = {
    { "SINE", WAVEFORM_TYPE_SINE },
    { "SQUARE", WAVEFORM_TYPE_SQUARE },
    { "TRIANGLE", WAVEFORM_TYPE_TRIANGLE },
    { "RAMP", WAVEFORM_TYPE_RAMP },
    { NULL, 0 },

    SCPI_CHOICE_LIST_END
};

//Implementation of Set Waveform Type Command
scpi_result_t SCPI_WaveFormSet(scpi_t *context) {
    int32_t param;
     if (!SCPI_ParamChoice(context, waveform_type, &param, TRUE)) {
        return SCPI_RES_ERR;
    }
    awg.waveform_type = (waveform_type_t)param;  // Set waveform type
    return SCPI_RES_OK;
}   
//Implementation of Query Waveform Type Command
scpi_result_t SCPI_WaveFormQuery(scpi_t *context) {
    SCPI_ResultInt32(context, awg.waveform_type);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_DevNameSet(scpi_t *context) {
    char device_name[100];
    size_t length;

    if (!SCPI_ParamCopyText(context, device_name, sizeof(device_name), &length, TRUE)) {
        return SCPI_RES_ERR;
    }
    strncpy(awg.device_name, device_name, sizeof(awg.device_name));
    return SCPI_RES_OK;
}

//Implementation of Query Device Name Command
scpi_result_t SCPI_DevNameQuery(scpi_t *context) {
    const char *device_name = "My Device"; // Suppose this is the current device name
   if (awg.device_name[0] == '\0') {
        SCPI_Write(context,awg.device_name, strlen(awg.device_name));
    } else {
        SCPI_Write(context, awg.device_name, strlen(awg.device_name));
    }
    return SCPI_RES_OK;
}

//Load Custom Waveform Command
scpi_result_t  SCPI_LoadArbitraryWaveform(scpi_t *context){
    const char *data;
    size_t length;

    if (SCPI_ParamArbitraryBlock(context, &data, &length, TRUE)) {
        // load_waveform(data, length);  // Assume that load _ waveform is a function of loading waveform data
        //mmemdata
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

//Maximum, minimum, and default values for query frequency
scpi_result_t SCPI_FrequencyInstQ(scpi_t *context) {
    awg.frequency_max = 10000000.0;
    awg.frequency_min = 1000.0;
    awg.frequency_default = 1000000.0;
    char query_type[16];
    int32_t choice_val = 0;
    size_t length;

     static const scpi_choice_def_t freq_choices[] = {
        {"MAX", 1},
        {"MIN", 2},
        {"DEF", 3},
        {NULL, 0}
    };

     if (!SCPI_ParamChoice(context, freq_choices, &choice_val, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (choice_val == 1) {
        SCPI_ResultDouble(context, awg.frequency_max);
    } else if (choice_val == 2) {
        SCPI_ResultDouble(context, awg.frequency_min);
    } else if (choice_val == 3) {
        SCPI_ResultDouble(context, awg.frequency_default);
    } else {
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t SCPI_MemoryDataAppend(scpi_t *context){
    char file_name[MAX_FILE_NAME_LENGTH] = {0};
    const char *data;
    size_t data_length;

    if (!SCPI_ParamCopyText(context, file_name, sizeof(file_name), &data_length, TRUE)) {
         return SCPI_RES_ERR;
     }

    if (!SCPI_ParamArbitraryBlock(context, &data, &data_length, TRUE)) {
        return SCPI_RES_ERR; 
    }

    FILE *file = fopen(file_name, "a");
    if (file == NULL) {
        return SCPI_RES_ERR;
    }

    fwrite(data, 1, data_length, file);

    fclose(file);
    return SCPI_RES_OK;
}


// // //多参数命令
// static scpi_result_t SCPI_RunCommand(scpi_t *context) {
//     int32_t mode;
//     const char *trigger_source;

//     if (!SCPI_ParamInt32(context, &mode, TRUE)) {
//         return SCPI_RES_ERR;
//     }

//     if (!SCPI_ParamText(context, &trigger_source, FALSE)) {
//         return SCPI_RES_ERR;
//     }

//     // 执行命令
//     run(mode, trigger_source);  // 假设 run() 是实际执行的函数

//     return SCPI_RES_OK;
// }


// //Action commands without parameters
// static scpi_result_t SCPI_RunAction(scpi_t *context) {
//     run();  
//     return SCPI_RES_OK;
// }

scpi_result_t SCPI_MemoryDataSet(scpi_t *context) {
    char file_name[MAX_FILE_NAME_LENGTH] = {0};
    const char *data;
    size_t length;

    if(!SCPI_ParamCopyText(context, file_name, sizeof(file_name), &length, TRUE)) {
        return SCPI_RES_ERR;
    }
    if(!SCPI_ParamArbitraryBlock(context, &data, &length, TRUE)) {
        
        return SCPI_RES_ERR;
    }

    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        return SCPI_RES_ERR;
    }

    if (fwrite(data, 1, length, file) != length) {
        fclose(file);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t SCPI_MemoryDataQuery(scpi_t *context) {
   char file_name[MAX_FILE_NAME_LENGTH] = {0};
    size_t length;

    if (!SCPI_ParamCopyText(context, file_name, sizeof(file_name), &length, TRUE)) {
        return SCPI_RES_ERR;  
    }

   
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        return SCPI_RES_ERR;  
    }

    fseek(file, 0, SEEK_END); 
    long file_size = ftell(file); 
    fseek(file, 0, SEEK_SET); 

    if (file_size <= 0) {
        fclose(file);
        return SCPI_RES_ERR;  
    }

    char *buffer = (char *)malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        return SCPI_RES_ERR;  
    }

    fread(buffer, 1, file_size, file);  
    fclose(file); 


    SCPI_ResultArbitraryBlock(context, buffer, file_size);

    free(buffer);

    return SCPI_RES_OK;
}



scpi_result_t SCPI_SystemHelpHeaders(scpi_t *context) {
    int count = 0;
    size_t total_length = 0;

    for (const scpi_command_t *cmd = scpi_commands; cmd->pattern != NULL; ++cmd) {
        total_length += strlen(cmd->pattern) + 1; 
        count++;
    }

    char *buffer = (char *)malloc(total_length + 1); 
    if (!buffer) return SCPI_RES_ERR;

    buffer[0] = '\0';
    for (const scpi_command_t *cmd = scpi_commands; cmd->pattern != NULL; ++cmd) {
        strcat(buffer, cmd->pattern);
        strcat(buffer, "\n");
    }

    SCPI_ResultArbitraryBlock(context, buffer, strlen(buffer));

    free(buffer);
    return SCPI_RES_OK;
}
