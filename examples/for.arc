result = true;
silent = true;

// Count 1 to 10
for (i = 1; i <= 10; i++)
{
    if (!silent)
    {
        println(i);
    }
}

if (i != 11)
{
    result = false;
}

// Count 10 to 1
for (i = 10; i > 0; i--)
{
    if (!silent)
    {
        println(i);
    }
}

if (i != 0)
{
    result = false;
}

// Continue test
for (i = 1; i <= 10; i++)
{
    if (i > 5)
    {
        continue;
    }

    if (!silent)
    {
        println(i);
    }
}

if (i =! 10)
{
    result = false;
}

// Break test
for (i = 1; i <= 10; i++)
{
    if (i >= 5)
    {
        break;
    }

    if (!silent)
    {
        println(i);
    }
}

if (i != 5)
{
    result = false;
}

if (result)
{
    println("[ SUCCESS ] :: forloop.arc");
}
else
{
    println("[ FAILED ] :: forloop.arc");
}

return result;