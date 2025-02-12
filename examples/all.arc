// This is a test
buf = "Hello";
buf += ", ";
buf += "ADDED WITH+=";
buf = buf + "World";
print(buf); 
if (buf == "Hello, World") { print("Strings are equal"); } 
a = 5; 
a = a + 2; 
if (a == 7) { print("a is 7"); } 
else if (a == 8) { print("a is 8"); } 
else { print("a is something else"); } 
a = a * 2; 
print("a = " + a);
b = 2; 
print(b); 
z = 5;
if (z != 5) { print("z is not 5"); } else { print("z is 5"); } 
if (z > 5) { print("z is > 5"); } 
if (z >= 5) { print("z is >= 5"); } 
if (z > 15) { print("z is > 15"); } 
if (z < 10) { print("z is < 10"); } 
print("a = " + a); 
a++;
print("a = " + a); 
a--;
print("a = " + a); 
print("FOR LOOP TEST");
i = 0;
for ( i = 0; i < 10; i = i + 1 ) { 
    print(i); 
}
for ( i = 10; i > 0; i-- ) { 
    print(i); 
}
b = true; 
if (b == true) { print("b is true"); } 
if (b == false) { print("b is false"); } 
c = false;
if (c == false) { print("if (c) is false"); } 
if (!c) { print("if (!c) is false"); } 
if ( (a == 5) && (b == true) ) { print("Both conditions met"); }
if ( (a != 5) || (!b) ) { print("At least one condition is met"); }
name = "Lucy";
if (name == "Lucy" || name == "Blake") { print("Hello, " + name); }
i = 0; 
b = false;
while ( i < 10 ) { 
    b = !b; 
    if (b) { print(i); }
    i = i + 1; 
} 
print("TYPEOF TESTS"); 
x = 42; 
s = "hello"; 
b = true; 
print(typeof(x));
print(typeof(s));
print(typeof(b));
name = "Blake Pell";
print(left(name, 5));
print(right(name, 4));
print(left(name, 100));
print("name is " + len(name) + " characters long.");
for ( i = 10; i > 0; i-- ) { 
    if (i > 5) { continue; } print(i);
}
i = 0;
while ( i < 10 ) { 
    i++; if (i > 5) { continue; } print(i);
}
for ( i = 100; i < 150; i++ ) { 
    if (i > 110) { break; } print(i);
}
while ( i > 0 ) { 
    i--; if (i < 100) { break; } print(i);
}
name = input("Enter your name: ");
print("You entered " + name);
print(is_number(name));
print(typeof(is_number(name)));
if (is_number(name)) { print("You entered a number"); } else { print("You did not enter a number"); }
b = 5;
print("You entered ${name} as your name.  The value of x is ${x}");

newVal = cstr(b);
print(typeof(newVal));
newVal += "0";
print(newVal);
newInt = cint(newVal);
print(typeof(newInt));
print(newInt);
newInt += 5;
print(newInt);

return true;
