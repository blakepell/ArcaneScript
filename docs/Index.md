# Arcane Script Documentation

## Table of Contents

- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Syntax Overview](syntax-overview.md)
- - [Keywords](keywords.md)
  - [Statements](statements.md)
  - [Expressions](#expressions.md)
  - [Variable Types](types.md)
  - [Operators](operators.md)
    - [Assignment Operators](operators.md#assignment-operators)
    - [Arithmetic Operators](operators.md#arithmetic-operators)
    - [Equality Operators](operators.md#equality-operators)
    - [Increment/Decrement Operators](operators.md#incrementdecrement-operators)
    - [Logical Operators](operators.md#logical-operators)
    - [Relational Operators](operators.md#relational-operators)
  - [Functions and Interop](interop.md)
  - [String Templates](string-templates.md)
- [Examples and Tutorials](#examples-and-tutorials) TODO
- [License](license.md)

# Introduction

Arcane is a lightweight, embeddable scripting language designed to be integrated into existing C code bases. It is an interpreted language which provides a minimal set of features with the goal of simplicity and ease of integration. Some key points:

- **Integration Focus:**  
  Arcane is intended to be compiled directly into your project. Its source is included in your code base, allowing you to execute scripts within your application without the need for external dependencies.

- **C Interop:**  
  The language itself does not support user-defined functions. Instead, it leverages C interop, allowing you to define functions in C and register them so that they can be invoked from Arcane scripts.

- **Single Interpreter Limitation:**  
 At this time, only one script instance can run at a time. Support for multiple interpreters is a planned feature for future enhancements.

- **Lightweight and Simple:**  
  Arcane focuses on providing essential scripting capabilities. Its design emphasizes ease of reading, simplicity, and minimal overhead, making it ideal for embedding.

Arcane is perfect for adding scripting support to applications where a full-blown language would be overkill and tight integration with C functions is required.

[Back to Index](index.md)