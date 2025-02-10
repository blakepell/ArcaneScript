result = true;
i = 10;
print(i);

if (i != 10)
{
    print("[ Failed ] i != 10");
    result = false;
}

i++;
print(i);

if (i != 11)
{
    print("[ Failed ] i != 11");
    result = false;
}

i--;
print(i);

if (i != 10)
{
    print("[ Failed ] i != 10");
    result = false;
}

return result;