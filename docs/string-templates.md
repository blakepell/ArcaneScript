# String Templates

String templates in Arcane allow you to embed variable references directly inside string literals. When processed by the interpreter, expressions enclosed in `${` and `}` are evaluated and replaced with their corresponding values.

## How It Works

- Enclose text in double quotes.
- Use the syntax `${expression}` to include variables or expressions.
- At runtime, the interpreter evaluates these expressions and concatenates their string representations with the rest of the text.

## Example Usage

```arcane
user = "Alice";
age = 30;
print("Hello, ${user}! You are ${age} years old.");
```

## Benefits

- Simplifies dynamic string construction.
- Improves readability by reducing explicit concatenation.
- Provides flexibility to mix static text with dynamic values.

[Back to Index](index.md)