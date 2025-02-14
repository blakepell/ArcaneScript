result = true;
i = 10;

if (i != 10)
{
    result = false;
}

i++;

if (i != 11)
{
    result = false;
}

i--;

if (i != 10)
{
    result = false;
}

if (result)
{
    println("[ SUCCESS ] :: postfix.arc");
}
else
{
    println("[ FAILED ] :: postfix.arc");
}

return result;