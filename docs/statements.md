# Statements

In Arcane, statements are the building blocks that control the flow of program execution. The interpreter (as defined in arcane.c) handles several statement types:

- **Print Statement:**  
  Outputs an expression to the console.  
  Example:
  ```C
  println("Hello, World!");
  ```

- **Return Statement:**  
  Exits the current execution flow and optionally returns a value.  
  Example:
  ```C
  return 42;
  ```

- **Conditional Statement (if/else):**  
  Evaluates a condition and executes a block of statements if true. An optional `else` block can run when the condition is false.
  ```C
  if (x > 0) {
      println("Positive");
  } else {
      println("Non-positive");
  }
  ```

- **Loop Statements:**  
  - **For Loop:**  
    Executes a block repeatedly with initializer, condition, and post-expression.
    ```C
    for (i = 0; i < 10; i += 1) {
        println(i);
    }
    ```C
  - **While Loop:**  
    Continues execution as long as a condition remains true.
    ```arcane
    while (x > 0) {
        println(x);
        x -= 1;
    }
    ```

- **Control Flow Statements:**  
  - **Continue:** Skips to the next iteration of a loop.
  - **Break:** Exits the current loop immediately.

These statement forms are parsed by the `parse_statement` function in arcane.c. Each statement must end with a semicolon where required, and blocks are enclosed within braces `{}`.

[Back to Index](index.md)
