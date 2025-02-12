# Expressions

Expressions in Arcane are constructs that evaluate to a value. They are parsed and computed by a chain of functions in arcane.c:
- `parse_primary` handles literals (numbers, strings, booleans) and variable references.
- `parse_unary` deals with prefix operators (like `!`, `++`, `--`).
- `parse_term` and `parse_factor` handle arithmetic operations and concatenation.
- `parse_assignment` processes assignments (e.g., `x = expr` or `x += expr`).

## Expression Types

- **Literals:** Direct values such as `42`, `"Hello"`, or `true`.
- **Arithmetic Expressions:** Combine literals and variables with operators:
  ```arcane
  3 + 4 * 2 - 1;
  ```
- **Variable References:** Use variable names to retrieve stored values:
  ```arcane
  x;
  ```
- **Function Calls (Interop):** Although Arcane does not support user-defined functions, calling interop functions is allowed:
  ```arcane
  typeof(x);
  ```

## How Expressions Are Processed

When an expression is encountered:
- The interpreter invokes the corresponding parsing function based on the operator precedence.
- Each function (such as `parse_primary` or `parse_term`) evaluates a part of the expression.
- Operators and their operands are processed in sequence, yielding a final value.

Expressions must adhere to the syntax defined in arcane.c, ensuring correct operator usage and valid tokens.

[Back to Index](index.md)
