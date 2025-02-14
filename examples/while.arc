i = 0;
buf = "";

while (i < 1000)
{
    i++;

    if (is_interval(i, 100))
    {
        println(i);
    }
}

if (i == 1000)
{
    result = true;
    println("[ SUCCESS ] :: while.arc");
}
else
{
    result = false;
    println("[ FAILED ] :: while.arc");
}

return result;