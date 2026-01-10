# SCPI Unit Tests with Google Test

This directory contains example C++ unit tests using Google Test (GTest) to demonstrate how to test SCPI commands with various parameter types.

## Test Files

### `scpi_basic_example.cpp`
A simple, minimal example showing basic SCPI parameter types:
- **Integer (int32)**: `INT 42` / `INT?`
- **Double**: `DOUBle 3.14159` / `DOUBle?`
- **Boolean**: `BOOL ON` / `BOOL?`
- **String**: `STRing "Hello"` / `STRing?`

This is the best starting point for understanding how to write SCPI tests.

### `scpi_parameter_test.cpp`
Comprehensive test suite demonstrating all major SCPI parameter types:
- **Signed/Unsigned 32-bit integers**: `TEST:INT32`, `TEST:UINT32`
- **Signed/Unsigned 64-bit integers**: `TEST:INT64`, `TEST:UINT64`
- **Double precision floating point**: `TEST:DOUBle`
- **Boolean values**: `TEST:BOOL` (ON/OFF, 1/0)
- **String parameters**: `TEST:STRing`
- **Choice parameters**: `TEST:CHOice` (OFF/ON/AUTO)
- Error handling tests

### `scpi_advanced_test.cpp`
Advanced SCPI features:
- **Arrays**: `TEST:ARRay:INT32`, `TEST:ARRay:DOUBle`
- **Float**: `TEST:FLOat`
- **Multiple parameters**: `TEST:MULTi`
- **Arbitrary blocks**: `TEST:BLOCk` (basic structure)

## Building and Running Tests

The tests are automatically included when you build the project. To run them:

```bash
# Build the project
cmake --build build

# Run all tests
ctest --test-dir build

# Run specific test executable
./build/libscpi/test/UnitTest/scpi_parser_utest
```

Or using Google Test directly:

```bash
./build/libscpi/test/UnitTest/scpi_parser_utest --gtest_filter=SCPIBasicExample.*
```

## Writing Your Own Tests

### Basic Structure

1. **Create a test fixture class** inheriting from `::testing::Test`:
```cpp
class MySCPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SCPI context
    }
    
    scpi_t context;
    // ... buffers and handlers
};
```

2. **Implement SCPI interface callbacks**:
```cpp
static size_t WriteHandler(scpi_t* ctx, const char* data, size_t len) {
    // Capture output
    return len;
}

static int ErrorHandler(scpi_t* ctx, int_fast16_t err) {
    // Track errors
    return 0;
}
```

3. **Define command handlers**:
```cpp
static scpi_result_t MyCommand(scpi_t* ctx) {
    int32_t value;
    if (!SCPI_ParamInt32(ctx, &value, TRUE)) {
        return SCPI_RES_ERR;
    }
    // Process value...
    return SCPI_RES_OK;
}
```

4. **Register commands**:
```cpp
const scpi_command_t commands[] = {
    {.pattern = "MY:COMMand", .callback = MyCommand},
    SCPI_CMD_LIST_END
};
```

5. **Write test cases**:
```cpp
TEST_F(MySCPITest, TestMyCommand) {
    std::string result = execute("MY:COMMand 42\r\n");
    EXPECT_FALSE(hasError());
    // Assertions...
}
```

## Parameter Types Reference

### Integer Parameters
- `SCPI_ParamInt32(context, &value, mandatory)` - 32-bit signed integer
- `SCPI_ParamUInt32(context, &value, mandatory)` - 32-bit unsigned integer
- `SCPI_ParamInt64(context, &value, mandatory)` - 64-bit signed integer
- `SCPI_ParamUInt64(context, &value, mandatory)` - 64-bit unsigned integer

### Floating Point Parameters
- `SCPI_ParamFloat(context, &value, mandatory)` - 32-bit float
- `SCPI_ParamDouble(context, &value, mandatory)` - 64-bit double

### Boolean Parameters
- `SCPI_ParamBool(context, &value, mandatory)` - Boolean (ON/OFF, 1/0, TRUE/FALSE)

### String Parameters
- `SCPI_ParamCopyText(context, buffer, size, &len, mandatory)` - Copy text to buffer
- `SCPI_ParamCharacters(context, &ptr, &len, mandatory)` - Get pointer to characters

### Choice Parameters
- `SCPI_ParamChoice(context, options, &value, mandatory)` - Select from choice list

### Array Parameters
- `SCPI_ParamArrayInt32(context, data, max_count, &count, format, mandatory)`
- `SCPI_ParamArrayDouble(context, data, max_count, &count, format, mandatory)`
- Similar for other types

### Result Functions
- `SCPI_ResultInt32(context, value)` - Return integer
- `SCPI_ResultDouble(context, value)` - Return double
- `SCPI_ResultBool(context, value)` - Return boolean
- `SCPI_ResultText(context, text)` - Return text string
- `SCPI_ResultArrayInt32(context, array, count, format)` - Return array

## Example Command Patterns

```cpp
// Simple set command
TEST:VOLTage 5.0

// Query command
TEST:VOLTage?

// Multiple parameters
TEST:CONFig 100,3.14,ON

// Array parameter
TEST:ARRay 1,2,3,4,5

// String parameter
TEST:NAME "Device Name"

// Choice parameter
TEST:MODE AUTO
```

## Notes

- All SCPI commands should end with `\r\n` (carriage return + line feed)
- Commands can be chained with semicolons: `CMD1;CMD2;CMD3\r\n`
- Query commands return values that can be captured in the write handler
- Error handling is important - always check return values from parameter functions
- The `mandatory` parameter determines if missing parameters cause errors
