#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "arcane.h"

/* ============================================================
   Example Script (main)
   ============================================================ */

   int main(void) {
    /* Example script demonstrating:
         - String variables and concatenation (using both + and +=)
         - Integer arithmetic and assignment
         - Equality operators in if/else if/else
         - The return statement
         - The print statement
         - Function calls (inc() and foo())
         - All statements end with semicolons.
    */
    const char *script =
        "// This is a test\n"
        "buf = \"Hello\"; "
        "buf += \", \"; "
        "buf = buf + \"World\"; "
        "print(buf); "
        "if (buf == \"Hello, World\") { print(\"Strings are equal\"); } "
        "a = 5; "
        "a = a + 2; "
        "if (a == 7) { print(\"a is 7\"); } "
        "else if (a == 8) { print(\"a is 8\"); } "
        "else { print(\"a is something else\"); } "
        "a = a * 2; "
        "print(\"a = \" + a);"
        "b = inc(a); "
        "print(b); "
        "print(foo()); "
        "z = 5;"
        "if (z != 5) { print(\"z is not 5\"); } else { print(\"z is 5\"); } "
        "if (z > 5) { print(\"z is > 5\"); } "
        "if (z >= 5) { print(\"z is >= 5\"); } "
        "if (z > 15) { print(\"z is > 15\"); } "
        "if (z < 10) { print(\"z is < 10\"); } "
        "print(\"a = \" + a); "
        "a++;"
        "print(\"a = \" + a); "
        "a--;"
        "print(\"a = \" + a); "
        "print(\"FOR LOOP TEST\");"
        "i = 0;\n"
        "for ( i = 0; i < 10; i = i + 1 ) { "
        "    print(i); "
        "}"
        "for ( i = 10; i > 0; i-- ) { "
        "    print(i); "
        "}"
        "b = true; "
        "if (b == true) { print(\"b is true\"); } "
        "if (b == false) { print(\"b is false\"); } "
        "c = false;"
        "if (c == false) { print(\"if (c) is false\"); } "
        "if (!c) { print(\"if (!c) is false\"); } "     
        "if ( (a == 5) && (b == true) ) { print(\"Both conditions met\"); }"
        "if ( (a != 5) || (!b) ) { print(\"At least one condition is met\"); }"
        "name = \"Lucy\";"
        "if (name == \"Lucy\" || name == \"Blake\") { print(\"Hello, \" + name); }"
        "i = 0; "
        "b = false;"
        "while ( i < 10 ) { "
        "    b = !b; "
        "    if (b) { print(i); }"
        "    i = i + 1; "
        "} "        
        "return 1; ";
        
    Value ret = interpret(script);
    printf("Script returned: ");
    if(ret.type == VAL_INT)
        printf("%d\n", ret.int_val);
    else if(ret.type == VAL_STRING)
        printf("%s\n", ret.str_val);
    else
        printf("null\n");
    if(ret.type == VAL_STRING && ret.temp)
        free_value(ret);
    return 0;
}