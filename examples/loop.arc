i = 0;
buf = "";

for (i = 0; i < 100000000; i++)
{
    if (is_interval(i, 1000000))
    {
        println(i);
    }
}

println("Done!");