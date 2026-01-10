# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SCPI-Parser is a C library (v2.1.0) for parsing SCPI (Standard Commands for Programmable Instruments) commands on the instrument side. It implements SCPI-99 (IEEE 488.2-2004 compliant) and supports short/long form command aliases, compound commands, queries, and various parameter types.

## Build Commands

**Using Make (recommended):**
```bash
make clean all    # Clean and build library and examples
make test         # Run all unit tests
make install      # Install library headers and binaries
```

**Using CMake:**
```bash
mkdir build && cd build
conan install .. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build .
ctest  # Run tests
```

## Architecture

The library follows a lexer-parser pattern with callback-based command dispatch:

```
lexer.c → tokenizes SCPI commands into tokens
parser.c → matches tokens against command patterns and invokes callbacks
```

### Key Components

- **Lexer** ([lexer.c](libscpi/src/lexer.c)): Tokenizes input into token types (SCPI_TOKEN_COMMA, SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA, etc.)
- **Parser** ([parser.c](libscpi/src/parser.c)): Matches command patterns, manages parameter extraction, and calls registered callbacks
- **IEEE 488.2 registers** ([ieee488.c](libscpi/src/ieee488.c)): Status Byte, ESR, OPER, QUES registers
- **Error queue** ([error.c](libscpi/src/error.c), [fifo.c](libscpi/src/fifo.c)): FIFO-based error queue
- **Unit system** ([units.c](libscpi/src/units.c)): 50+ unit types (Volts, Amps, Ohms, Hz, etc.)

### Core Data Types

- `scpi_t` ([types.h:424](libscpi/inc/scpi/types.h#L424)): Main context structure containing command list, buffer, error queue, registers
- `scpi_command_t` ([types.h:408](libscpi/inc/scpi/types.h#L408)): Command pattern + callback function + optional tag
- `scpi_parameter_t` / `scpi_param_list_t`: Parsed parameters available to callbacks
- `scpi_error_t` ([types.h:276](libscpi/inc/scpi/types.h#L276)): Error code and optional device-dependent info

### Defining Commands

Commands are defined as arrays of `scpi_command_t` terminated with `SCPI_CMD_LIST_END`:

```c
static scpi_result_t cmd_VOLTAGE(scpi_t *ctx) {
    int32_t value;
    if (SCPI_ParamInt(ctx, &value, TRUE)) {
        // Handle value
    }
    return SCPI_RES_OK;
}

static const scpi_command_t scpi_commands[] = {
    { "VOLTage", cmd_VOLTAGE },
    { "VOLTage:RANGe", cmd_VOLTAGE_RANGE },
    SCPI_CMD_LIST_END
};
```

Callbacks receive `scpi_t*` context and return `scpi_result_t`. Use `SCPI_Param*` functions to extract parameters.

### Result Functions

Send responses using `SCPI_Result*` functions:
- `SCPI_ResultDouble(ctx, val)` - Send numeric response
- `SCPI_ResultText(ctx, "text")` - Send string response
- `SCPI_ResultArbitraryBlock(ctx, data, len)` - Send binary data

## Configuration

Key settings in [config.h](libscpi/inc/scpi/config.h):

- `SCPI_LINE_ENDING`: CR, LF, or CRLF termination (default CRLF)
- `SYSTEM_TYPE`: `SYSTEM_BARE_METAL` (0) or `SYSTEM_FULL_BLOWN` (1) - affects error list size
- `USE_FULL_ERROR_LIST`: Include full error messages (default: enabled on full systems)
- `USE_MEMORY_ALLOCATION_FREE`: Use static heap for bare-metal (0) or malloc/free (1)
- `USE_UNITS_*`: Enable/disable unit types (Voltage, Frequency, etc.) for code size

For bare-metal systems, create `scpi_user_config.h` with custom overrides.

## Test Framework

Unit tests use CUnit. Test files in [libscpi/test/](libscpi/test/):
- `test_fifo.c`, `test_lexer_parser.c`, `test_parser.c`, `test_scpi_utils.c`

Tests are built as `<test_name>.test` binaries in the test directory.
