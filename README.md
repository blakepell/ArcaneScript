# Arcane Script Documentation

## Introduction

Arcane is an embeddable scripting language designed to be integrated into existing C code bases. It is an interpreted language which provides a minimal set of features with the goal of simplicity and ease of integration.

## Table of Contents

- [Introduction](#introduction)
- [Getting Started](docs/getting-started.md)
- [Syntax Overview](docs/syntax-overview.md)
- - [Keywords](docs/keywords.md)
  - [Statements](docs/statements.md)
  - [Expressions](docs/expressions.md)
  - [Variable Types](docs/types.md)
  - [Operators](docs/operators.md)
    - [Assignment Operators](docs/operators.md#assignment-operators)
    - [Arithmetic Operators](docs/operators.md#arithmetic-operators)
    - [Equality Operators](docs/operators.md#equality-operators)
    - [Increment/Decrement Operators](docs/operators.md#incrementdecrement-operators)
    - [Logical Operators](docs/operators.md#logical-operators)
    - [Relational Operators](docs/operators.md#relational-operators)
  - [Functions and Interop](docs/interop.md)
  - [String Templates](docs/string-templates.md)
- [Examples](docs/examples)
- [License](LICENSE)

- **Integration Focus:**  
  Arcane is designed to be integrated into your project as source code. Its inclusion in your codebase allows for the execution of scripts without relying on external dependencies. The provided release PowerShell script generates an amalgamated source file specifically for this purpose. Furthermore, as an alternative the release script produces a .EXE for running Arcane script files and a .DLL for interfacing with other programs. An example .NET 9 application in this repository demonstrates how to interoperate with Arcane.

- **C Interop:**  
  The language itself does not support user-defined functions. Instead, it leverages C interop, allowing you to define functions in C and register them so that they can be invoked from Arcane scripts.

- **Single Interpreter Limitation:**  
 At this time, only one script instance can run at a time. Support for multiple interpreters is a planned feature for future enhancements.

- **Lightweight and Simple:**  
  Arcane focuses on providing essential scripting capabilities. Its design emphasizes ease of reading, simplicity, and minimal overhead, making it ideal for embedding.

Arcane is perfect for adding scripting support to applications where a full-blown language would be overkill and tight integration with C functions is required.

[Back to Index](index.md)