# Types

Arcane supports a limited set of types as defined in the interpreter:

- **Integer (VAL_INT)**  
  Represents whole numbers. Operations like addition, subtraction, multiplication, and division work on this type.

- **String (VAL_STRING)**  
  Represents sequences of characters. Used for text manipulation and supports string templates for dynamic interpolation.

- **Boolean (VAL_BOOL)**  
  Represents truth values (`true` and `false`). Used in logical expressions and conditions.

- **Double (VAL_DOUBLE)**  
  Represents a double-precision floating-point.

- **Double (VAL_DATE)**  
  Represents a date only format: `YYYY/MM/DD` or `MM/DD/YYYY`

- **Array (VAL_ARRAY)**  
  Represents an array of `Value` objects.  This means that an array is not type specific as a `Value` holds other primitive data types (including arrays).

- **Null (VAL_NULL)**  
  Represents an absence of value. Often used as a default or error indicator.

[Back to Index](index.md)
