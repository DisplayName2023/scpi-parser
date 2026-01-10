/*
 * Basic SCPI Test Example
 * Simple demonstration of how to test SCPI commands with different parameter types
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include "scpi/scpi.h"
#include "scpi/units.h"

// Simple test fixture
class SCPIBasicExample : public ::testing::Test {
protected:
    void SetUp() override {
        output_buffer[0] = '\0';
        output_pos = 0;
        error_count = 0;
        
        // Setup interface
        interface.error = ErrorHandler;
        interface.write = WriteHandler;
        interface.control = ControlHandler;
        interface.flush = FlushHandler;
        interface.reset = ResetHandler;
        
        // Initialize SCPI
        SCPI_Init(&context, commands, &interface, get_scpi_units_def(),
                  "TEST", "MFG", "MODEL", "1.0",
                  input_buffer, sizeof(input_buffer),
                  error_queue, static_cast<int16_t>(sizeof(error_queue)/sizeof(error_queue[0])));
        
        SCPI_ErrorClear(&context);
    }
    
    std::string execute(const char* cmd) {
        output_buffer[0] = '\0';
        output_pos = 0;
        error_count = 0;
        SCPI_Input(&context, cmd, strlen(cmd));
        return std::string(output_buffer);
    }
    
    // Storage
    static int32_t stored_int;
    static double stored_double;
    static scpi_bool_t stored_bool;
    static char stored_string[256];
    
    // Buffers
    static char output_buffer[1024];
    static size_t output_pos;
    static int error_count;
    
    scpi_t context;
    char input_buffer[256];
    scpi_error_t error_queue[4];
    scpi_interface_t interface;
    
    // Handlers
    static size_t WriteHandler(scpi_t* ctx, const char* data, size_t len) {
        (void)ctx;
        if (output_pos + len < sizeof(output_buffer)) {
            memcpy(output_buffer + output_pos, data, len);
            output_pos += len;
            output_buffer[output_pos] = '\0';
        }
        return len;
    }
    
    static scpi_result_t FlushHandler(scpi_t* ctx) {
        (void)ctx;
        return SCPI_RES_OK;
    }
    
    static int ErrorHandler(scpi_t* ctx, int_fast16_t err) {
        (void)ctx;
        (void)err;
        error_count++;
        return 0;
    }
    
    static scpi_result_t ControlHandler(scpi_t* ctx, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
        (void)ctx;
        (void)ctrl;
        (void)val;
        return SCPI_RES_OK;
    }
    
    static scpi_result_t ResetHandler(scpi_t* ctx) {
        (void)ctx;
        return SCPI_RES_OK;
    }
    
    // Command handlers
    static scpi_result_t SetInt(scpi_t* ctx) {
        return SCPI_ParamInt32(ctx, &stored_int, TRUE) ? SCPI_RES_OK : SCPI_RES_ERR;
    }
    
    static scpi_result_t GetInt(scpi_t* ctx) {
        SCPI_ResultInt32(ctx, stored_int);
        return SCPI_RES_OK;
    }
    
    static scpi_result_t SetDouble(scpi_t* ctx) {
        return SCPI_ParamDouble(ctx, &stored_double, TRUE) ? SCPI_RES_OK : SCPI_RES_ERR;
    }
    
    static scpi_result_t GetDouble(scpi_t* ctx) {
        SCPI_ResultDouble(ctx, stored_double);
        return SCPI_RES_OK;
    }
    
    static scpi_result_t SetBool(scpi_t* ctx) {
        return SCPI_ParamBool(ctx, &stored_bool, TRUE) ? SCPI_RES_OK : SCPI_RES_ERR;
    }
    
    static scpi_result_t GetBool(scpi_t* ctx) {
        SCPI_ResultBool(ctx, stored_bool);
        return SCPI_RES_OK;
    }
    
    static scpi_result_t SetString(scpi_t* ctx) {
        size_t len;
        return SCPI_ParamCopyText(ctx, stored_string, sizeof(stored_string), &len, TRUE) 
               ? SCPI_RES_OK : SCPI_RES_ERR;
    }
    
    static scpi_result_t GetString(scpi_t* ctx) {
        SCPI_ResultText(ctx, stored_string);
        return SCPI_RES_OK;
    }
    
    static const scpi_command_t commands[];
};

// Static definitions
int32_t SCPIBasicExample::stored_int = 0;
double SCPIBasicExample::stored_double = 0.0;
scpi_bool_t SCPIBasicExample::stored_bool = FALSE;
char SCPIBasicExample::stored_string[256] = "";
char SCPIBasicExample::output_buffer[1024] = "";
size_t SCPIBasicExample::output_pos = 0;
int SCPIBasicExample::error_count = 0;

const scpi_command_t SCPIBasicExample::commands[] = {
    {.pattern = "*IDN?", .callback = SCPI_CoreIdnQ},
    {.pattern = "INT", .callback = SetInt},
    {.pattern = "INT?", .callback = GetInt},
    {.pattern = "DOUBle", .callback = SetDouble},
    {.pattern = "DOUBle?", .callback = GetDouble},
    {.pattern = "BOOL", .callback = SetBool},
    {.pattern = "BOOL?", .callback = GetBool},
    {.pattern = "STRing", .callback = SetString},
    {.pattern = "STRing?", .callback = GetString},
    SCPI_CMD_LIST_END
};

// Example tests
TEST_F(SCPIBasicExample, ExampleInt) {
    std::string result = execute("INT 42\r\n");
    EXPECT_EQ(stored_int, 42);
    EXPECT_EQ(error_count, 0);
    
    result = execute("INT?\r\n");
    EXPECT_STREQ(result.c_str(), "42\r\n");
}

TEST_F(SCPIBasicExample, ExampleDouble) {
    std::string result = execute("DOUBle 3.14159\r\n");
    EXPECT_DOUBLE_EQ(stored_double, 3.14159);
    
    result = execute("DOUBle?\r\n");
    EXPECT_NE(result.find("3.14159"), std::string::npos);
}

TEST_F(SCPIBasicExample, ExampleBool) {
    std::string result = execute("BOOL ON\r\n");
    EXPECT_EQ(stored_bool, TRUE);
    
    result = execute("BOOL?\r\n");
    EXPECT_STREQ(result.c_str(), "1\r\n");
    
    result = execute("BOOL OFF\r\n");
    EXPECT_EQ(stored_bool, FALSE);
    
    result = execute("BOOL?\r\n");
    EXPECT_STREQ(result.c_str(), "0\r\n");
}

TEST_F(SCPIBasicExample, ExampleString) {
    std::string result = execute("STRing \"Hello\"\r\n");
    EXPECT_STREQ(stored_string, "Hello");
    
    result = execute("STRing?\r\n");
    EXPECT_STREQ(result.c_str(), "\"Hello\"\r\n");
}
