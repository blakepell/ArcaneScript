print("Count 1 to 10");
for (i = 1; i <= 10; i++)
{
    print(i);
}

print ("Count 10 to 1");
for (i = 10; i > 0; i--)
{
    print(i);
}

print ("Continue test");
for (i = 1; i <= 10; i++)
{
    if (i > 5)
    {
        continue;
    }

    print(i);
}

if (i == 10)
{
    print("Continue test passed.");
}

print ("Continue test");
for (i = 1; i <= 10; i++)
{
    if (i >= 5)
    {
        break;
    }

    print(i);    
}

if (i == 5)
{
    print("Break test passed");
}

return true;