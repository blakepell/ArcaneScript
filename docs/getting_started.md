# Getting Started

To integrate Arcane into your project, make sure you include the following files in your build:
- `arcane.c`
- `arcane.h`
- `functions.c`

These files provide the scripting language interpreter and the interop functionality needed to call C functions from scripts.

## Example Usage

Include the header and call the `interpret` function to execute a script:

```c
#include "arcane.h"

int main(void) {
    // Your Arcane script as a C string
    const char *script = "print(\"Hello, Arcane!\");";
    
    // Execute the script
    Value result = interpret(script);
    
    // Optionally process the result, if needed
    free_value(result);

    return 0;
}
```

This simple example demonstrates how to send a script to the interpreter. For more details, refer to the other sections of this documentation.

[Back to Index](index.md)
