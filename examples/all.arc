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
name = "250";
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

for (i = 1; i < 10; i++)
{
    println(rnd(1, 100));
}

for (i = 1; i < 10; i++)
{
    println(chance(50));
}

name = "Blake Brandon Pell";
new_name = replace(name, " Brandon ", " ");
println("${name} => ${new_name}");

str = "  This is a string with space at the start and end ";
println(len(str));
println(str);

str = trim(str);
println(len(str));
println(str);

before = "  Blake ";
after = trim_start(before);
println(after);

before = "  Blake ";
after = trim_end(before);
println(after);

println(ucase("blake"));
println(lcase("BLAKE"));

max = umax(7, 10);
println("${max} is the larger of 7 and 10.");

min = umin(7, 10);
println("${min} is the smaller of 7 and 10.");

t = timestr();
println("The current date/time is: ${t}");

z = -15;
z = abs(z);

println("The absolute value of -15 is ${z}");

dbl = 0.5;
dbl = dbl + 0.25;

// Doesn't work
println("Double + " + dbl);

// Works
println("Double = ${dbl}");

dbl = round_up(dbl);
println("${dbl} should be 1");

dbl = 0.75;
dbl = round_down(dbl);
println("${dbl} (should be 0)");

dbl = 0.75;
dbl = round(dbl);
println("${dbl} (should be 1");

x = 64.0;
x = sqrt(64.0);
println("Square root of 64 is ${x}");

buf = "My name is Blake!!!";

if (contains(buf, "Blake"))
{
    println("String contained Blake");
}

if (!contains(buf, "Hrmph"))
{
    println("String didn't contain Hrmph!");
}

if (ends_with(buf, "Blake"))
{
    println("The string ended with Blake");
}

if (starts_with(buf, "Blake"))
{
    println("The string ended with Blake");
}
else
{
    println("The string did not start with Blake");
}

println(index_of(buf, "Blake", 0));
println(index_of(buf, "Bill", 0));
println(buf);

println(last_index_of(buf, "Blake", len(buf)));
println(last_index_of(buf, "Bill", 0));

str_val = "4.45";
d = cdbl(str_val);
d = round_down(d);

println(d);

bday = cdate("01/27/1977");
m = month(bday);
y = year(bday);

println("${m}/${y}");

d = cdate("1/1/1980");

if (bday < d) 
{
    println("You're old!");
}

dt = today();
println("${dt}");
println(dt);
println(" ");

println(cepoch(bday));

bday = add_years(bday, 1);
bday = add_months(bday, 4);
bday = add_days(bday, -9);

println("New date: ${bday}");

num = cepoch(bday);
newDt = cdate(num);
println(newDt);

println(terminal_height());
println(terminal_width());

print(chr(13));
print(chr(10));
print(chr(13));
print(chr(10));

print(asc(" "));

// Split a string and iterate over its parts

s = "apple,banana,cherry";       // The string to split
arr = split(s, ",");             // Split the string by comma

ub = upperbound(arr);            // Get the last valid index in the array

// Loop from index 0 up to the upper bound and print each element.
for (i = 0; i <= ub; i = i + 1) {
    println(arr[i]);
}

arr = new_array(10);

// Set the element at index 0 using array_set
array_set(arr, 0, "first element");
array_set(arr, 1, "second element");

// Retrieve and print the element to verify
println(arr[0]);
println(arr[1]);

println("END");
return true;
