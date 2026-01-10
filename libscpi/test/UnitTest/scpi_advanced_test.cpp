/*
 * Google Test example for advanced SCPI features
 * Demonstrates arrays, arbitrary blocks, expressions, and other advanced SCPI types
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "scpi/scpi.h"
#include "scpi/units.h"

// Test fixtures for advanced SCPI features
class SCPIAdvancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize output buffer
        output_buffer[0] = '\0';
        output_buffer_pos = 0;
        
        // Initialize error buffer
        err_buffer_pos = 0;
        
        // Setup SCPI interface
        scpi_interface.error = SCPI_Error;
        scpi_interface.write = SCPI_Write;
        scpi_interface.control = SCPI_Control;
        scpi_interface.flush = SCPI_Flush;
        scpi_interface.reset = SCPI_Reset;
        
        // Initialize SCPI context
        SCPI_Init(&scpi_context,
                  scpi_commands,
                  &scpi_interface,
                  get_scpi_units_def(),
                  "TEST", "MANUFACTURER", "MODEL", "VERSION",
                  scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
                  scpi_error_queue_data, static_cast<int16_t>(SCPI_ERROR_QUEUE_SIZE));
        
        // Clear any errors
        SCPI_ErrorClear(&scpi_context);
        
        // Clear test arrays
        test_int32_array.clear();
        test_double_array.clear();
        test_arbitrary_block.clear();
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
    
    // Helper to execute SCPI command and get output
    std::string executeCommand(const char* cmd) {
        output_buffer_clear();
        error_buffer_clear();
        SCPI_Input(&scpi_context, cmd, strlen(cmd));
        return std::string(output_buffer);
    }
    
    // Helper to check if error occurred
    bool hasError() {
        return err_buffer_pos > 0;
    }
    
    void output_buffer_clear() {
        output_buffer[0] = '\0';
        output_buffer_pos = 0;
    }
    
    void error_buffer_clear() {
        err_buffer_pos = 0;
        SCPI_ErrorClear(&scpi_context);
    }
    
    // Storage for test values
    static std::vector<int32_t> test_int32_array;
    static std::vector<double> test_double_array;
    static std::vector<uint8_t> test_arbitrary_block;
    static float test_float_value;
    
    // Output buffer
    static char output_buffer[2048];
    static size_t output_buffer_pos;
    
    // Error buffer
    static int_fast16_t err_buffer[128];
    static size_t err_buffer_pos;
    
    // SCPI context
    scpi_t scpi_context;
    
    // Input buffer
    static constexpr size_t SCPI_INPUT_BUFFER_LENGTH = 512;
    static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
    
    // Error queue
    static constexpr size_t SCPI_ERROR_QUEUE_SIZE = 4;
    static scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
    
    // SCPI interface
    scpi_interface_t scpi_interface;
    
    // Callback functions
    static size_t SCPI_Write(scpi_t* context, const char* data, size_t len) {
        (void)context;
        if (output_buffer_pos + len < sizeof(output_buffer)) {
            memcpy(output_buffer + output_buffer_pos, data, len);
            output_buffer_pos += len;
            output_buffer[output_buffer_pos] = '\0';
        }
        return len;
    }
    
    static scpi_result_t SCPI_Flush(scpi_t* context) {
        (void)context;
        return SCPI_RES_OK;
    }
    
    static int SCPI_Error(scpi_t* context, int_fast16_t err) {
        (void)context;
        if (err_buffer_pos < 128) {
            err_buffer[err_buffer_pos++] = err;
        }
        return 0;
    }
    
    static scpi_result_t SCPI_Control(scpi_t* context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
        (void)context;
        (void)ctrl;
        (void)val;
        return SCPI_RES_OK;
    }
    
    static scpi_result_t SCPI_Reset(scpi_t* context) {
        (void)context;
        return SCPI_RES_OK;
    }
    
    // Command handlers for arrays
    static scpi_result_t TestInt32ArraySet(scpi_t* context) {
        int32_t data[100];
        size_t count = 0;
        
        if (!SCPI_ParamArrayInt32(context, data, 100, &count, SCPI_FORMAT_ASCII, TRUE)) {
            return SCPI_RES_ERR;
        }
        
        test_int32_array.assign(data, data + count);
        return SCPI_RES_OK;
    }
    
    static scpi_result_t TestInt32ArrayQuery(scpi_t* context) {
        if (test_int32_array.empty()) {
            SCPI_ResultInt32(context, 0);
            return SCPI_RES_OK;
        }
        
        SCPI_ResultArrayInt32(context, test_int32_array.data(), 
                              test_int32_array.size(), SCPI_FORMAT_ASCII);
        return SCPI_RES_OK;
    }
    
    static scpi_result_t TestDoubleArraySet(scpi_t* context) {
        double data[100];
        size_t count = 0;
        
        if (!SCPI_ParamArrayDouble(context, data, 100, &count, SCPI_FORMAT_ASCII, TRUE)) {
            return SCPI_RES_ERR;
        }
        
        test_double_array.assign(data, data + count);
        return SCPI_RES_OK;
    }
    
    static scpi_result_t TestDoubleArrayQuery(scpi_t* context) {
        if (test_double_array.empty()) {
            SCPI_ResultDouble(context, 0.0);
            return SCPI_RES_OK;
        }
        
        SCPI_ResultArrayDouble(context, test_double_array.data(), 
                               test_double_array.size(), SCPI_FORMAT_ASCII);
        return SCPI_RES_OK;
    }
    
    // Command handler for arbitrary block
    static scpi_result_t TestArbitraryBlockSet(scpi_t* context) {
        const char* data;
        size_t len;
        
        if (!SCPI_ParamArbitraryBlock(context, &data, &len, TRUE)) {
            return SCPI_RES_ERR;
        }
        
        test_arbitrary_block.assign(reinterpret_cast<const uint8_t*>(data), 
                                     reinterpret_cast<const uint8_t*>(data) + len);
        return SCPI_RES_OK;
    }
    
    static scpi_result_t TestArbitraryBlockQuery(scpi_t* context) {
        if (test_arbitrary_block.empty()) {
            SCPI_ResultArbitraryBlock(context, nullptr, 0);
            return SCPI_RES_OK;
        }
        
        SCPI_ResultArbitraryBlock(context, test_arbitrary_block.data(), 
                                  test_arbitrary_block.size());
        return SCPI_RES_OK;
    }
    
    // Command handler for float
    static scpi_result_t TestFloatSet(scpi_t* context) {
        if (!SCPI_ParamFloat(context, &test_float_value, TRUE)) {
            return SCPI_RES_ERR;
        }
        return SCPI_RES_OK;
    }
    
    static scpi_result_t TestFloatQuery(scpi_t* context) {
        SCPI_ResultFloat(context, test_float_value);
        return SCPI_RES_OK;
    }
    
    // Command handler for multiple parameters
    static scpi_result_t TestMultiParamSet(scpi_t* context) {
        int32_t int_val;
        double double_val;
        scpi_bool_t bool_val;
        
        if (!SCPI_ParamInt32(context, &int_val, TRUE)) {
            return SCPI_RES_ERR;
        }
        if (!SCPI_ParamDouble(context, &double_val, TRUE)) {
            return SCPI_RES_ERR;
        }
        if (!SCPI_ParamBool(context, &bool_val, TRUE)) {
            return SCPI_RES_ERR;
        }
        
        // Store values (using static variables for simplicity)
        test_int32_array.clear();
        test_int32_array.push_back(int_val);
        test_double_array.clear();
        test_double_array.push_back(double_val);
        test_bool_value = bool_val;
        
        return SCPI_RES_OK;
    }
    
    // Command handler for JSON-like string parameter
    static scpi_result_t TestJsonString(scpi_t* context) {
        const char* json_str;
        size_t len;

        if (!SCPI_ParamCharacters(context, &json_str, &len, TRUE)) {
            return SCPI_RES_ERR;
        }

        test_json_string.assign(json_str, len);
        return SCPI_RES_OK;
    }

    static scpi_result_t TestJsonStringQuery(scpi_t* context) {
        if (test_json_string.empty()) {
            SCPI_ResultText(context, "");
            return SCPI_RES_OK;
        }
        SCPI_ResultText(context, test_json_string.c_str());
        return SCPI_RES_OK;
    }

    static std::string test_json_string;

    static scpi_bool_t test_bool_value;
    
    // Command list
    static const scpi_command_t scpi_commands[];
};

// Static member definitions
std::vector<int32_t> SCPIAdvancedTest::test_int32_array;
std::vector<double> SCPIAdvancedTest::test_double_array;
std::vector<uint8_t> SCPIAdvancedTest::test_arbitrary_block;
float SCPIAdvancedTest::test_float_value = 0.0f;
scpi_bool_t SCPIAdvancedTest::test_bool_value = FALSE;
char SCPIAdvancedTest::output_buffer[2048] = "";
size_t SCPIAdvancedTest::output_buffer_pos = 0;
int_fast16_t SCPIAdvancedTest::err_buffer[128] = {0};
size_t SCPIAdvancedTest::err_buffer_pos = 0;
char SCPIAdvancedTest::scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH] = "";
scpi_error_t SCPIAdvancedTest::scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE] = {};
std::string SCPIAdvancedTest::test_json_string;

// Command definitions
const scpi_command_t SCPIAdvancedTest::scpi_commands[] = {
    // IEEE Mandated Commands
    {.pattern = "*CLS", .callback = SCPI_CoreCls},
    {.pattern = "*IDN?", .callback = SCPI_CoreIdnQ},
    {.pattern = "*RST", .callback = SCPI_CoreRst},
    
    // Advanced test commands
    {.pattern = "TEST:ARRay:INT32", .callback = TestInt32ArraySet},
    {.pattern = "TEST:ARRay:INT32?", .callback = TestInt32ArrayQuery},
    {.pattern = "TEST:ARRay:DOUBle", .callback = TestDoubleArraySet},
    {.pattern = "TEST:ARRay:DOUBle?", .callback = TestDoubleArrayQuery},
    {.pattern = "TEST:BLOCk", .callback = TestArbitraryBlockSet},
    {.pattern = "TEST:BLOCk?", .callback = TestArbitraryBlockQuery},
    {.pattern = "TEST:FLOat", .callback = TestFloatSet},
    {.pattern = "TEST:FLOat?", .callback = TestFloatQuery},
    {.pattern = "TEST:MULTi", .callback = TestMultiParamSet},
    {.pattern = "TEST:SCPI", .callback = TestJsonString},
    {.pattern = "TEST:SCPI?", .callback = TestJsonStringQuery},

    SCPI_CMD_LIST_END
};

// Array tests
TEST_F(SCPIAdvancedTest, TestInt32ArraySingle) {
    std::string result = executeCommand("TEST:ARRay:INT32 42\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_int32_array.size(), 1);
    EXPECT_EQ(test_int32_array[0], 42);
}

TEST_F(SCPIAdvancedTest, TestInt32ArrayMultiple) {
    std::string result = executeCommand("TEST:ARRay:INT32 1,2,3,4,5\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_int32_array.size(), 5);
    EXPECT_EQ(test_int32_array[0], 1);
    EXPECT_EQ(test_int32_array[1], 2);
    EXPECT_EQ(test_int32_array[2], 3);
    EXPECT_EQ(test_int32_array[3], 4);
    EXPECT_EQ(test_int32_array[4], 5);
}

TEST_F(SCPIAdvancedTest, TestInt32ArrayNegative) {
    std::string result = executeCommand("TEST:ARRay:INT32 -10,-20,-30\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_int32_array.size(), 3);
    EXPECT_EQ(test_int32_array[0], -10);
    EXPECT_EQ(test_int32_array[1], -20);
    EXPECT_EQ(test_int32_array[2], -30);
}

TEST_F(SCPIAdvancedTest, TestInt32ArrayQuery) {
    test_int32_array = {100, 200, 300};
    std::string result = executeCommand("TEST:ARRay:INT32?\r\n");
    EXPECT_FALSE(hasError());
    // Result should contain the array values
    EXPECT_NE(result.find("100"), std::string::npos);
    EXPECT_NE(result.find("200"), std::string::npos);
    EXPECT_NE(result.find("300"), std::string::npos);
}

// Double array tests
TEST_F(SCPIAdvancedTest, TestDoubleArraySingle) {
    std::string result = executeCommand("TEST:ARRay:DOUBle 3.14\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_double_array.size(), 1);
    EXPECT_DOUBLE_EQ(test_double_array[0], 3.14);
}

TEST_F(SCPIAdvancedTest, TestDoubleArrayMultiple) {
    std::string result = executeCommand("TEST:ARRay:DOUBle 1.1,2.2,3.3,4.4,5.5\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_double_array.size(), 5);
    EXPECT_DOUBLE_EQ(test_double_array[0], 1.1);
    EXPECT_DOUBLE_EQ(test_double_array[1], 2.2);
    EXPECT_DOUBLE_EQ(test_double_array[2], 3.3);
    EXPECT_DOUBLE_EQ(test_double_array[3], 4.4);
    EXPECT_DOUBLE_EQ(test_double_array[4], 5.5);
}

TEST_F(SCPIAdvancedTest, TestDoubleArrayScientific) {
    std::string result = executeCommand("TEST:ARRay:DOUBle 1E-3,2E-2,3E-1\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_double_array.size(), 3);
    EXPECT_NEAR(test_double_array[0], 0.001, 1e-9);
    EXPECT_NEAR(test_double_array[1], 0.02, 1e-9);
    EXPECT_NEAR(test_double_array[2], 0.3, 1e-9);
}

// Float tests
TEST_F(SCPIAdvancedTest, TestFloatPositive) {
    std::string result = executeCommand("TEST:FLOat 123.456\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_FLOAT_EQ(test_float_value, 123.456f);
}

TEST_F(SCPIAdvancedTest, TestFloatNegative) {
    std::string result = executeCommand("TEST:FLOat -99.99\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_FLOAT_EQ(test_float_value, -99.99f);
}

TEST_F(SCPIAdvancedTest, TestFloatQuery) {
    test_float_value = 42.5f;
    std::string result = executeCommand("TEST:FLOat?\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_NE(result.find("42.5"), std::string::npos);
}

// Multiple parameter tests
TEST_F(SCPIAdvancedTest, TestMultipleParameters) {
    std::string result = executeCommand("TEST:MULTi 100,3.14,ON\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_int32_array.size(), 1);
    EXPECT_EQ(test_int32_array[0], 100);
    EXPECT_EQ(test_double_array.size(), 1);
    EXPECT_DOUBLE_EQ(test_double_array[0], 3.14);
    EXPECT_EQ(test_bool_value, TRUE);
}

TEST_F(SCPIAdvancedTest, TestMultipleParametersMixed) {
    std::string result = executeCommand("TEST:MULTi -50,2.718,OFF\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_int32_array[0], -50);
    EXPECT_DOUBLE_EQ(test_double_array[0], 2.718);
    EXPECT_EQ(test_bool_value, FALSE);
}

// Arbitrary block tests (basic structure - actual implementation may vary)
TEST_F(SCPIAdvancedTest, TestArbitraryBlockBasic) {
    // Note: Arbitrary block format is #<num_digits><length><data>
    // This is a simplified test - actual format may be more complex
    std::string result = executeCommand("TEST:BLOCk #14Test\r\n");
    // The parsing depends on the exact format expected
    // This test demonstrates the structure
    EXPECT_FALSE(hasError() || hasError()); // May or may not error depending on format
}

// JSON-like string test
TEST_F(SCPIAdvancedTest, TestJsonString) {
    // SCPI uses double quotes with escaped double quotes inside: " ""
    std::string result = executeCommand("TEST:SCPI \"{ \"\"Voltage\"\", 1.0, \"\"Mode\"\", \"\"A\"\" }\"\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_EQ(test_json_string, R"({ ""Voltage"", 1.0, ""Mode"", ""A"" })");
}

TEST_F(SCPIAdvancedTest, TestJsonStringQuery) {
    test_json_string = "{ \"Voltage\", 1.0, \"Mode\", \"A\" }";
    std::string result = executeCommand("TEST:SCPI?\r\n");
    EXPECT_FALSE(hasError());
    EXPECT_NE(result.find(R"({ ""Voltage"", 1.0, ""Mode"", ""A"" })"), std::string::npos);
}

TEST_F(SCPIAdvancedTest, TestJsonStringEmpty) {
    test_json_string = "";
    std::string result = executeCommand("TEST:SCPI?\r\n");
    EXPECT_FALSE(hasError());
    // Empty string query should return empty quotes ""
    EXPECT_NE(result.find("\"\""), std::string::npos);
}
