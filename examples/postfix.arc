result = true;
i = 10;
println(i);

if (i != 10)
{
    println("[ Failed ] i != 10");
    result = false;
}

i++;
println(i);

if (i != 11)
{
    println("[ Failed ] i != 11");
    result = false;
}

i--;
println(i);

if (i != 10)
{
    println("[ Failed ] i != 10");
    result = false;
}

return result;