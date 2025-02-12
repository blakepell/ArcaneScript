println("Count 1 to 10");
for (i = 1; i <= 10; i++)
{
    println(i);
}

println ("Count 10 to 1");
for (i = 10; i > 0; i--)
{
    println(i);
}

println ("Continue test");
for (i = 1; i <= 10; i++)
{
    if (i > 5)
    {
        continue;
    }

    println(i);
}

if (i == 10)
{
    println("Continue test passed.");
}

println ("Continue test");
for (i = 1; i <= 10; i++)
{
    if (i >= 5)
    {
        break;
    }

    println(i);    
}

if (i == 5)
{
    println("Break test passed");
}

return true;