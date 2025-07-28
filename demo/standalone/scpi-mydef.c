#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "scpi/scpi.h"
#include "scpi/units.h"
#include "scpi-def.h"
#include "awg.h"

// Note: For the lexer functions used in My_SCPI_ParamTranslateNumberVal,
// we would need access to lexer_private.h, but since this is a demo,
// we'll declare the functions we need:
extern int scpiLex_DecimalNumericProgramData(lex_state_t * state, scpi_token_t * token);
extern int scpiLex_WhiteSpace(lex_state_t * state, scpi_token_t * token);
extern int scpiLex_SuffixProgramData(lex_state_t * state, scpi_token_t * token);

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

// Extended Parameters Handling Example with Units
// This demonstrates advanced parameter validation and unit conversion
scpi_result_t SCPI_FrequencySetAdvanced(scpi_t *context) {
    scpi_number_t freq_param;
    scpi_parameter_t param;
    
    // Define special values for frequency (MIN, MAX, DEF)
    static const scpi_choice_def_t freq_special[] = {
        {"MIN", SCPI_NUM_MIN},
        {"MAX", SCPI_NUM_MAX}, 
        {"DEF", SCPI_NUM_DEF},
        {NULL, 0}
    };
    
    // First, check if we have a valid parameter
    if (!SCPI_Parameter(context, &param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    // Check if the parameter is a valid number or special value
    if (!SCPI_ParamIsValid(&param)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    
    // Check if this is a number parameter (including units)
    if (SCPI_ParamIsNumber(context, &param)) {
        // Parse the number with units using extended parameter handling
        if (!SCPI_ParamNumber(context, freq_special, &freq_param, TRUE)) {
            return SCPI_RES_ERR;
        }
        
        // Handle special values
        if (freq_param.special) {
            switch (freq_param.content.tag) {
                case SCPI_NUM_MIN:
                    awg.frequency = awg.frequency_min;
                    break;
                case SCPI_NUM_MAX:
                    awg.frequency = awg.frequency_max;
                    break;
                case SCPI_NUM_DEF:
                    awg.frequency = awg.frequency_default;
                    break;
                default:
                    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                    return SCPI_RES_ERR;
            }
        } else {
            // Handle numeric values with units
            double frequency_value = freq_param.content.value;
            
                         // Validate unit compatibility
             if (freq_param.unit != SCPI_UNIT_NONE && freq_param.unit != SCPI_UNIT_HERTZ) {
                 SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
                 return SCPI_RES_ERR;
             }
            
            // The unit conversion is already handled by SCPI_ParamNumber
            // freq_param.content.value contains the value in base units (Hz)
            
            // Validate frequency range
            if (frequency_value < awg.frequency_min || frequency_value > awg.frequency_max) {
                SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
                return SCPI_RES_ERR;
            }
            
            awg.frequency = frequency_value;
        }
    } else {
        // Try alternative parameter conversion methods
        double freq_val;
        
        // Use SCPI_ParamToDouble for direct conversion
        if (SCPI_ParamToDouble(context, &param, &freq_val)) {
            if (freq_val < awg.frequency_min || freq_val > awg.frequency_max) {
                SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
                // return SCPI_RES_ERR;
            }
            awg.frequency = freq_val;
        } else {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_TYPE_ERROR);
            return SCPI_RES_ERR;
        }
    }
    
    return SCPI_RES_OK;
}

// Query frequency with unit formatting
scpi_result_t SCPI_FrequencyQueryAdvanced(scpi_t *context) {
    scpi_number_t result_number;
    char result_buffer[64];
    size_t len;
    
    // Prepare the number structure for output with units
    result_number.special = FALSE;
    result_number.content.value = awg.frequency;
    result_number.unit = SCPI_UNIT_HERTZ;
    result_number.base = 10;
    
    // Use SCPI_NumberToStr to format the number with units
    len = SCPI_NumberToStr(context, scpi_special_numbers_def, &result_number, result_buffer, sizeof(result_buffer));
    
    if (len > 0) {
        SCPI_ResultCharacters(context, result_buffer, len);
    } else {
        // Fallback to simple double output
        SCPI_ResultDouble(context, awg.frequency);
    }
    
    return SCPI_RES_OK;
}



// Extended parameter handling for voltage with multiple parameter validation
scpi_result_t SCPI_VoltageSetExtended(scpi_t *context) {
    scpi_number_t voltage_param;
    scpi_parameter_t param;
    
    // Get the parameter reference
    if (!SCPI_Parameter(context, &param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    // Validate parameter before processing
    if (!SCPI_ParamIsValid(&param)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    
    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &voltage_param, TRUE)) {
        if (voltage_param.unit == SCPI_UNIT_HERTZ) {
            printf("Setting frequency to %.2f MHz\n", voltage_param.content.value);
            // Device-specific code to set frequency
            return SCPI_RES_OK;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return SCPI_RES_ERR;
        }
    }

    // Check if it's a number parameter
    if (!SCPI_ParamIsNumber(context, &param)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_TYPE_ERROR);
        return SCPI_RES_ERR;
    }
    
    // Parse number with unit support
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &voltage_param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    // Handle regular numeric values
    if (!voltage_param.special) {
        double voltage_value = voltage_param.content.value;
        
                 // Validate unit (should be voltage or unitless)
         if (voltage_param.unit != SCPI_UNIT_NONE && voltage_param.unit != SCPI_UNIT_VOLT) {
             SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
             return SCPI_RES_ERR;
         }
        
        // Apply reasonable voltage limits (example: ±10V)
        if (voltage_value < -10.0 || voltage_value > 10.0) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return SCPI_RES_ERR;
        }
        
        awg.amplitude = voltage_value;
        return SCPI_RES_OK;
    } else {
        // Handle special values (MIN, MAX, DEF, etc.)
        switch (voltage_param.content.tag) {
            case SCPI_NUM_MIN:
                awg.amplitude = -10.0;  // Minimum voltage
                break;
            case SCPI_NUM_MAX:
                awg.amplitude = 10.0;   // Maximum voltage
                break;
            case SCPI_NUM_DEF:
                awg.amplitude = 1.0;    // Default voltage
                break;
            default:
                SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                return SCPI_RES_ERR;
        }
    }
    
    return SCPI_RES_OK;
}

// Manual Unit Handling Example - When you detect SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA_WITH_SUFFIX
// This shows low-level unit processing for educational purposes
scpi_result_t SCPI_ManualUnitHandling(scpi_t *context) {
    scpi_parameter_t param;
    double numeric_value;
    
    // Get the parameter
    if (!SCPI_Parameter(context, &param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    // Check if this is a numeric parameter with suffix (units)
    if (param.type == SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA_WITH_SUFFIX) {
        // Method 1: Use the high-level SCPI_ParamNumber API (RECOMMENDED)
        // This automatically handles unit conversion
        scpi_number_t number_with_units;
        if (SCPI_ParamNumber(context, scpi_special_numbers_def, &number_with_units, TRUE)) {
            // number_with_units.content.value now contains the value in base units
            // number_with_units.unit contains the unit type (SCPI_UNIT_VOLT, SCPI_UNIT_HERTZ, etc.)
            
            awg.amplitude = number_with_units.content.value;
            return SCPI_RES_OK;
        }
        
    } else if (param.type == SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA) {
        // Handle numeric data without units
        if (SCPI_ParamToDouble(context, &param, &numeric_value)) {
            awg.amplitude = numeric_value;
            return SCPI_RES_OK;
        }
    } else if (param.type == SCPI_TOKEN_PROGRAM_MNEMONIC) {
        // Handle special values like MIN, MAX, DEF
        int32_t choice_tag;
        if (SCPI_ParamToChoice(context, &param, scpi_special_numbers_def, &choice_tag)) {
            switch (choice_tag) {
                case SCPI_NUM_MIN:
                    awg.amplitude = -10.0;
                    break;
                case SCPI_NUM_MAX:
                    awg.amplitude = 10.0;
                    break;
                case SCPI_NUM_DEF:
                    awg.amplitude = 1.0;
                    break;
                default:
                    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                    return SCPI_RES_ERR;
            }
            return SCPI_RES_OK;
        }
    }
    
    SCPI_ErrorPush(context, SCPI_ERROR_DATA_TYPE_ERROR);
    return SCPI_RES_ERR;
}

// Advanced Manual Unit Processing - Shows internal lexer usage (ADVANCED/EDUCATIONAL)
// This replicates what SCPI_ParamNumber does internally
scpi_result_t SCPI_AdvancedManualUnitHandling(scpi_t *context) {
    scpi_parameter_t param;
    lex_state_t state;
    scpi_token_t suffix_token;
    double base_value;
    scpi_unit_t detected_unit = SCPI_UNIT_NONE;
    double multiplier = 1.0;
    
    // Get the parameter
    if (!SCPI_Parameter(context, &param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    // Only handle parameters with suffix
    if (param.type != SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA_WITH_SUFFIX) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_TYPE_ERROR);
        return SCPI_RES_ERR;
    }
    
    // Extract the numeric value first (this strips the suffix)
    if (!SCPI_ParamToDouble(context, &param, &base_value)) {
        return SCPI_RES_ERR;
    }
    
    // Set up lexer state to parse the suffix manually
    state.buffer = param.ptr;
    state.pos = state.buffer;
    state.len = param.len;
    
    // Skip the numeric part to get to the suffix
    // Note: This is simplified - the real implementation uses scpiLex functions
    // Find where the number ends and suffix begins
    char *suffix_start = param.ptr;
    while (suffix_start < param.ptr + param.len && 
           (*suffix_start == '.' || *suffix_start == '-' || *suffix_start == '+' || 
            (*suffix_start >= '0' && *suffix_start <= '9') || 
            *suffix_start == 'e' || *suffix_start == 'E')) {
        suffix_start++;
    }
    
    // Skip whitespace
    while (suffix_start < param.ptr + param.len && 
           (*suffix_start == ' ' || *suffix_start == '\t')) {
        suffix_start++;
    }
    
    // Extract suffix
    size_t suffix_len = (param.ptr + param.len) - suffix_start;
    
    if (suffix_len > 0) {
        // Manual unit lookup - checking common units
        // In real implementation, use translateUnit() function from units.c
        
        if (strncmp(suffix_start, "V", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_VOLT;
            multiplier = 1.0;
        } else if (strncmp(suffix_start, "MV", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_VOLT;
            multiplier = 1e-3;
        } else if (strncmp(suffix_start, "UV", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_VOLT;
            multiplier = 1e-6;
        } else if (strncmp(suffix_start, "KV", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_VOLT;
            multiplier = 1e3;
        } else if (strncmp(suffix_start, "HZ", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_HERTZ;
            multiplier = 1.0;
        } else if (strncmp(suffix_start, "KHZ", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_HERTZ;
            multiplier = 1e3;
        } else if (strncmp(suffix_start, "MHZ", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_HERTZ;
            multiplier = 1e6;
        } else if (strncmp(suffix_start, "GHZ", suffix_len) == 0) {
            detected_unit = SCPI_UNIT_HERTZ;
            multiplier = 1e9;
        } else {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return SCPI_RES_ERR;
        }
        
        // Apply the multiplier to convert to base units
        base_value *= multiplier;
        
        // Validate unit compatibility based on context
        if (detected_unit != SCPI_UNIT_VOLT && detected_unit != SCPI_UNIT_NONE) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return SCPI_RES_ERR;
        }
    }
    
    // Range validation
    if (base_value < -10.0 || base_value > 10.0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    
    // Store the converted value
    awg.amplitude = base_value;
    
    return SCPI_RES_OK;
}

// ================================================================================================
// COMPREHENSIVE GUIDE: How to Use scpi_number_t (_scpi_number_parameter_t) Structure
// ================================================================================================

/*
Structure Definition:
struct _scpi_number_parameter_t {
    scpi_bool_t special;     // TRUE if special value (MIN, MAX, DEF), FALSE if numeric
    union {
        double value;        // Numeric value (when special == FALSE)
        int32_t tag;        // Special value tag (when special == TRUE)
    } content;
    scpi_unit_t unit;       // Unit type (SCPI_UNIT_VOLT, SCPI_UNIT_HERTZ, etc.)
    int8_t base;           // Number base (10 for decimal, 16 for hex, etc.)
};
typedef struct _scpi_number_parameter_t scpi_number_t;
*/

// Example 1: Complete scpi_number_t Usage - Reading and Processing
scpi_result_t SCPI_NumberStructureDemo(scpi_t *context) {
    scpi_number_t number_param;
    
    // Parse parameter using SCPI_ParamNumber
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &number_param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    // Check if it's a special value or numeric value
    if (number_param.special) {
        // Handle special values (MIN, MAX, DEF, etc.)
        printf("Special value detected, tag: %d\n", number_param.content.tag);
        
        switch (number_param.content.tag) {
            case SCPI_NUM_MIN:
                awg.frequency = awg.frequency_min;
                printf("Setting to minimum frequency: %f Hz\n", awg.frequency);
                break;
                
            case SCPI_NUM_MAX:
                awg.frequency = awg.frequency_max;
                printf("Setting to maximum frequency: %f Hz\n", awg.frequency);
                break;
                
            case SCPI_NUM_DEF:
                awg.frequency = awg.frequency_default;
                printf("Setting to default frequency: %f Hz\n", awg.frequency);
                break;
                
            case SCPI_NUM_UP:
                awg.frequency *= 1.1;  // Increase by 10%
                printf("Stepping frequency up to: %f Hz\n", awg.frequency);
                break;
                
            case SCPI_NUM_DOWN:
                awg.frequency *= 0.9;  // Decrease by 10%
                printf("Stepping frequency down to: %f Hz\n", awg.frequency);
                break;
                
                
            case SCPI_NUM_AUTO:
                awg.frequency = awg.frequency_default;  // Auto mode
                printf("Auto mode - setting to default: %f Hz\n", awg.frequency);
                break;
                
            default:
                SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                return SCPI_RES_ERR;
        }
    } else {
        // Handle numeric values
        printf("Numeric value: %f, Unit: %d, Base: %d\n", 
               number_param.content.value, number_param.unit, number_param.base);
        
        // Check unit type
        switch (number_param.unit) {
            case SCPI_UNIT_NONE:
                printf("No unit specified - using raw value\n");
                awg.frequency = number_param.content.value;
                break;
                
            case SCPI_UNIT_HERTZ:
                printf("Frequency unit detected - value in Hz: %f\n", number_param.content.value);
                awg.frequency = number_param.content.value;
                break;
                
            case SCPI_UNIT_VOLT:
                printf("Voltage unit detected - value in V: %f\n", number_param.content.value);
                awg.amplitude = number_param.content.value;
                break;
                
            default:
                printf("Unsupported unit type: %d\n", number_param.unit);
                SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
                return SCPI_RES_ERR;
        }
        
        // Check number base
        switch (number_param.base) {
            case 10:
                printf("Decimal number\n");
                break;
            case 16:
                printf("Hexadecimal number\n");
                break;
            case 8:
                printf("Octal number\n");
                break;
            case 2:
                printf("Binary number\n");
                break;
            default:
                printf("Unknown number base: %d\n", number_param.base);
                break;
        }
    }
    
    return SCPI_RES_OK;
}

// Example 2: Creating and Initializing scpi_number_t for Output
scpi_result_t SCPI_NumberStructureOutput(scpi_t *context) {
    scpi_number_t output_number;
    char formatted_string[64];
    size_t len;
    
    // Method 1: Output a regular numeric value with units
    output_number.special = FALSE;              // Not a special value
    output_number.content.value = awg.frequency; // Set the numeric value
    output_number.unit = SCPI_UNIT_HERTZ;       // Set unit type
    output_number.base = 10;                    // Decimal base
    
    // Format the number with units using SCPI_NumberToStr
    len = SCPI_NumberToStr(context, scpi_special_numbers_def, &output_number, 
                          formatted_string, sizeof(formatted_string));
    
    if (len > 0) {
        printf("Formatted output: %s\n", formatted_string);
        SCPI_ResultCharacters(context, formatted_string, len);
    } else {
        // Fallback to simple output
        SCPI_ResultDouble(context, output_number.content.value);
    }
    
    return SCPI_RES_OK;
}

// Example 3: Advanced scpi_number_t Processing with Validation
scpi_result_t SCPI_NumberStructureAdvanced(scpi_t *context) {
    scpi_number_t number_param;
    double final_value;
    scpi_unit_t expected_unit = SCPI_UNIT_VOLT;  // Expecting voltage
    
    // Parse the parameter
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &number_param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    // Comprehensive processing
    if (number_param.special) {
        // Handle special values with custom logic
        switch (number_param.content.tag) {
            case SCPI_NUM_MIN:
                final_value = -10.0;  // Custom minimum
                break;
            case SCPI_NUM_MAX:
                final_value = 10.0;   // Custom maximum
                break;
            case SCPI_NUM_DEF:
                final_value = 0.0;    // Custom default
                break;
            default:
                SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                return SCPI_RES_ERR;
        }
        
        printf("Special value processed: tag=%d, result=%f\n", 
               number_param.content.tag, final_value);
    } else {
        // Process numeric values
        final_value = number_param.content.value;
        
        // Unit validation
        if (number_param.unit != SCPI_UNIT_NONE && number_param.unit != expected_unit) {
            printf("Unit mismatch: expected %d, got %d\n", expected_unit, number_param.unit);
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return SCPI_RES_ERR;
        }
        
        printf("Numeric value processed: value=%f, unit=%d, base=%d\n",
               final_value, number_param.unit, number_param.base);
    }
    
    // Range validation
    if (final_value < -100.0 || final_value > 100.0) {
        printf("Value out of range: %f (allowed: -100 to +100)\n", final_value);
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    
    // Store the processed value
    awg.amplitude = final_value;
    printf("Successfully set amplitude to: %f\n", final_value);
    
    return SCPI_RES_OK;
}

// Example 4: Manual Construction of scpi_number_t
scpi_result_t SCPI_NumberStructureConstruct(scpi_t *context) {
    scpi_number_t constructed_number;
    
    // Construct a numeric value with units
    constructed_number.special = FALSE;
    constructed_number.content.value = 1500000.0;  // 1.5 MHz in Hz
    constructed_number.unit = SCPI_UNIT_HERTZ;
    constructed_number.base = 10;
    
    // Use in processing or output
    awg.frequency = constructed_number.content.value;
    
    // Output the constructed number
    char output_buffer[32];
    size_t len = SCPI_NumberToStr(context, scpi_special_numbers_def, 
                                 &constructed_number, output_buffer, sizeof(output_buffer));
    
    if (len > 0) {
        SCPI_ResultCharacters(context, output_buffer, len);
    }
    
    return SCPI_RES_OK;
}

// ================================================================================================
// SCPI_ParamTranslateNumberVal - MISSING FUNCTION IMPLEMENTATION & DEMONSTRATION
// ================================================================================================

/*
NOTE: SCPI_ParamTranslateNumberVal is declared in libscpi/inc/scpi/units.h but NOT IMPLEMENTED
in the current codebase. This appears to be a planned function that was never completed.

Based on the function signature and name:
scpi_bool_t SCPI_ParamTranslateNumberVal(scpi_t * context, scpi_parameter_t * parameter);

The function would likely:
1. Take a parameter containing a number with units (like "1.5MHZ")
2. Translate/convert the units in-place
3. Return TRUE if successful, FALSE if unit translation failed

Here's what the implementation would likely look like:
*/

// Custom implementation of what SCPI_ParamTranslateNumberVal would do
scpi_bool_t My_SCPI_ParamTranslateNumberVal(scpi_t * context, scpi_parameter_t * parameter) {
    lex_state_t state;
    scpi_token_t numeric_token, suffix_token;
    double numeric_value;
    
    // Only process parameters with suffix (units)
    if (parameter->type != SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA_WITH_SUFFIX) {
        // No units to translate
        return TRUE;
    }
    
    // Set up lexer state
    state.buffer = parameter->ptr;
    state.pos = state.buffer;
    state.len = parameter->len;
    
    // Extract numeric part
    if (scpiLex_DecimalNumericProgramData(&state, &numeric_token) <= 0) {
        return FALSE;
    }
    
    // Skip whitespace
    scpiLex_WhiteSpace(&state, &suffix_token);
    
    // Extract suffix (unit)
    if (scpiLex_SuffixProgramData(&state, &suffix_token) <= 0) {
        return FALSE;
    }
    
    // Convert numeric part to double
    if (!SCPI_ParamToDouble(context, parameter, &numeric_value)) {
        return FALSE;
    }
    
    // Translate unit using the existing translateUnit function (from units.c)
    // Note: This requires access to internal functions, so this is a simplified version
    if (suffix_token.len > 0) {
        // Manual unit translation example
        if (strncmp(suffix_token.ptr, "MV", suffix_token.len) == 0) {
            numeric_value *= 1e-3;  // Convert millivolts to volts
        } else if (strncmp(suffix_token.ptr, "KV", suffix_token.len) == 0) {
            numeric_value *= 1e3;   // Convert kilovolts to volts
        } else if (strncmp(suffix_token.ptr, "MHZ", suffix_token.len) == 0) {
            numeric_value *= 1e6;   // Convert megahertz to hertz
        } else if (strncmp(suffix_token.ptr, "KHZ", suffix_token.len) == 0) {
            numeric_value *= 1e3;   // Convert kilohertz to hertz
        } else {
            // Unknown unit
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return FALSE;
        }
        
        // The translated value would be stored back somehow
        // (This is conceptual since the parameter structure is read-only)
        printf("Translated %.*s to base units: %f\n", 
               (int)parameter->len, parameter->ptr, numeric_value);
    }
    
    return TRUE;
}

// Demonstration of using SCPI_ParamTranslateNumberVal concept
scpi_result_t SCPI_ParamTranslateDemo(scpi_t *context) {
    scpi_parameter_t param;
    
    // Get the parameter
    if (!SCPI_Parameter(context, &param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    printf("Original parameter: %.*s (type: %d)\n", 
           (int)param.len, param.ptr, param.type);
    
    // Try to translate the parameter value
    if (My_SCPI_ParamTranslateNumberVal(context, &param)) {
        printf("Parameter translation successful\n");
        
        // Alternative: Use the high-level SCPI_ParamNumber instead
        // This is the RECOMMENDED approach since SCPI_ParamTranslateNumberVal doesn't exist
        scpi_number_t number_result;
        if (SCPI_ParamNumber(context, scpi_special_numbers_def, &number_result, TRUE)) {
            printf("RECOMMENDED: Using SCPI_ParamNumber instead\n");
            printf("Converted value: %f, Unit: %d, Base: %d\n",
                   number_result.content.value, number_result.unit, number_result.base);
            
            awg.amplitude = number_result.content.value;
        }
    } else {
        printf("Parameter translation failed\n");
        return SCPI_RES_ERR;
    }
    
    return SCPI_RES_OK;
}

// Better Alternative: Since SCPI_ParamTranslateNumberVal doesn't exist, 
// use existing functions to achieve the same result
scpi_result_t SCPI_ParamTranslateAlternative(scpi_t *context) {
    scpi_parameter_t param;
    scpi_number_t translated_number;
    
    // Get parameter
    if (!SCPI_Parameter(context, &param, TRUE)) {
        return SCPI_RES_ERR;
    }
    
    printf("Input parameter: %.*s\n", (int)param.len, param.ptr);
    
    // Method 1: Use SCPI_ParamNumber to get translated value
    if (SCPI_ParamNumber(context, scpi_special_numbers_def, &translated_number, TRUE)) {
        printf("Translated using SCPI_ParamNumber:\n");
        printf("  Value: %f (in base units)\n", translated_number.content.value);
        printf("  Unit type: %d\n", translated_number.unit);
        printf("  Number base: %d\n", translated_number.base);
        printf("  Special: %s\n", translated_number.special ? "YES" : "NO");
        
        // Store the translated value
        if (translated_number.unit == SCPI_UNIT_VOLT || translated_number.unit == SCPI_UNIT_NONE) {
            awg.amplitude = translated_number.content.value;
            printf("Set amplitude to: %f V\n", awg.amplitude);
        } else if (translated_number.unit == SCPI_UNIT_HERTZ) {
            awg.frequency = translated_number.content.value;
            printf("Set frequency to: %f Hz\n", awg.frequency);
        }
        
        return SCPI_RES_OK;
    }
    
    // Method 2: Manual parameter processing (if SCPI_ParamNumber fails)
    if (param.type == SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA_WITH_SUFFIX) {
        double value;
        if (SCPI_ParamToDouble(context, &param, &value)) {
            printf("Raw numeric value: %f (units not translated)\n", value);
            // Would need manual unit conversion here
        }
    }
    
    return SCPI_RES_ERR;
}

// Summary function showing best practices instead of SCPI_ParamTranslateNumberVal
scpi_result_t SCPI_ParamTranslateBestPractice(scpi_t *context) {
    printf("=== SCPI_ParamTranslateNumberVal DOES NOT EXIST ===\n");
    printf("Use these alternatives instead:\n\n");
    
    printf("1. RECOMMENDED: Use SCPI_ParamNumber() for automatic unit translation\n");
    printf("   scpi_number_t num;\n");
    printf("   if (SCPI_ParamNumber(context, scpi_special_numbers_def, &num, TRUE)) {\n");
    printf("       // num.content.value contains value in base units\n");
    printf("       // num.unit contains the unit type\n");
    printf("   }\n\n");
    
    printf("2. For manual control: Use SCPI_Parameter() + custom unit handling\n");
    printf("   scpi_parameter_t param;\n");
    printf("   if (SCPI_Parameter(context, &param, TRUE)) {\n");
    printf("       if (param.type == SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA_WITH_SUFFIX) {\n");
    printf("           // Extract and convert units manually\n");
    printf("       }\n");
    printf("   }\n\n");
    
    printf("3. Use existing extended parameter handling functions:\n");
    printf("   - SCPI_ParamIsValid()\n");
    printf("   - SCPI_ParamIsNumber()\n");
    printf("   - SCPI_ParamToDouble()\n");
    printf("   - SCPI_ParamToChoice()\n\n");
    
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
        return SCPI_RES_OK;
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