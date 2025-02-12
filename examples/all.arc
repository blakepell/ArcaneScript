// This is a test
buf = "Hello";
buf += ", ";
buf += "ADDED WITH+=";
buf = buf + "World";
println(buf); 
if (buf == "Hello, World") { println("Strings are equal"); } 
a = 5; 
a = a + 2; 
if (a == 7) { println("a is 7"); } 
else if (a == 8) { println("a is 8"); } 
else { println("a is something else"); } 
a = a * 2; 
println("a = " + a);
b = 2; 
println(b); 
z = 5;
if (z != 5) { println("z is not 5"); } else { println("z is 5"); } 
if (z > 5) { println("z is > 5"); } 
if (z >= 5) { println("z is >= 5"); } 
if (z > 15) { println("z is > 15"); } 
if (z < 10) { println("z is < 10"); } 
println("a = " + a); 
a++;
println("a = " + a); 
a--;
println("a = " + a); 
println("FOR LOOP TEST");
i = 0;
for ( i = 0; i < 10; i = i + 1 ) { 
    println(i); 
}
for ( i = 10; i > 0; i-- ) { 
    println(i); 
}
b = true; 
if (b == true) { println("b is true"); } 
if (b == false) { println("b is false"); } 
c = false;
if (c == false) { println("if (c) is false"); } 
if (!c) { println("if (!c) is false"); } 
if ( (a == 5) && (b == true) ) { println("Both conditions met"); }
if ( (a != 5) || (!b) ) { println("At least one condition is met"); }
name = "Lucy";
if (name == "Lucy" || name == "Blake") { println("Hello, " + name); }
i = 0; 
b = false;
while ( i < 10 ) { 
    b = !b; 
    if (b) { println(i); }
    i = i + 1; 
} 
println("TYPEOF TESTS"); 
x = 42; 
s = "hello"; 
b = true; 
println(typeof(x));
println(typeof(s));
println(typeof(b));
name = "Blake Pell";
println(left(name, 5));
println(right(name, 4));
println(left(name, 100));
println("name is " + len(name) + " characters long.");
for ( i = 10; i > 0; i-- ) { 
    if (i > 5) { continue; } println(i);
}
i = 0;
while ( i < 10 ) { 
    i++; if (i > 5) { continue; } println(i);
}
for ( i = 100; i < 150; i++ ) { 
    if (i > 110) { break; } println(i);
}
while ( i > 0 ) { 
    i--; if (i < 100) { break; } println(i);
}
name = input("Enter your name: ");
println("You entered " + name);
println(is_number(name));
println(typeof(is_number(name)));
if (is_number(name)) { println("You entered a number"); } else { println("You did not enter a number"); }
b = 5;
println("You entered ${name} as your name.  The value of x is ${x}");

newVal = cstr(b);
println(typeof(newVal));
newVal += "0";
println(newVal);
newInt = cint(newVal);
println(typeof(newInt));
println(newInt);
newInt += 5;
println(newInt);

name = "true";

if (cbool(name))
{
    println("${name} was true.");
}

for (i = 0; i < 1000; i++)
{
    if (is_interval(i, 100))
    {
        println(i);
    }
}

name = "b.pell";
println(substring(name, 2, 4));

return true;
