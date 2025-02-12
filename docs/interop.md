# Functions and Interop

In Arcane, the scripting language does not support user-defined functions. Instead, you can call functions implemented in C via interop. When the interpreter encounters a function call, it performs a lookup in a table of interop functions (using function pointers) to execute the corresponding C function.

## How It Works

1. **Declaration:**  
   Declare the function in the header file (e.g., in `arcane.h`).

2. **Definition:**  
   Implement the function in the source file (e.g., in `functions.c`).

3. **Registration:**  
   Add an entry for the function in the `interop_functions` table.  

## Example: Adding the 'typeof' Function

### Step 1: Declaration in arcane.h
```c
Value fn_typeof(Value *args, int arg_count);
```

### Step 2: Definition in arcane.c
```c
Value fn_typeof(Value *args, int arg_count) {
    if (arg_count != 1) {
        fprintf(stderr, "Runtime error: typeof expects exactly one argument.\n");
        exit(1);
    }

    Value arg = args[0];
    const char *type_str = "unknown";

    switch (arg.type) {
        case VAL_INT:
            type_str = "int";
            break;
        case VAL_STRING:
            type_str = "string";
            break;
        case VAL_BOOL:
            type_str = "bool";
            break;
        case VAL_NULL:
            type_str = "null";
            break;
        default:
            break;
    }

    return make_string(type_str);
}
```

### Step 3: Registration in the Interop Table
```c
static Function interop_functions[] = {
    {"typeof", fn_typeof},
    // ...existing function entries...
};
```

By following these steps, any new C function you implement can be made available to Arcane scripts through the interop mechanism.

[Back to Index](index.md)
