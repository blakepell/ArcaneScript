# Syntax Overview

Arcane's syntax is designed to be simple and familiar to C developers. Below is an overview of its basic constructs:

- **Statements:**  
  Statements must end with a semicolon (`;`). Code blocks are enclosed in `{}`.

- **Expressions:**  
  Expressions support arithmetic, logic, and string operations. Operator precedence is similar to C.

- **Control Structures:**  
  Uses standard `if`, `else`, `for`, and `while` constructs.

## Examples

### Basic Expression
```c
// Arithmetic and concatenation:
print(3 + 4 * 2 - 1);
```

### Variable Assignment and Usage
```c
// Assigning values to a variable (variable creation through assignment):
x = 10;
print(x);
```

### Conditional and Loop Statements
```c
// Using if/else and while-loop:
if (x > 5) {
    print("x is greater than 5");
} else {
    print("x is not greater than 5");
}

x = 5;

while (x > 0) {
    print(x);
    x -= 1;
}

for (x = 0; x < 5; x++) {
    print("Loop Iteration ${x}");
}
```

### Function Calls (Interop)
```c
// Calling an interop function:
result = typeof(x);
print(result);
```

These examples provide a quick look at the syntax rules of Arcane as defined in its parser. For more details, see the relevant documentation sections.

[Back to Index](index.md)
