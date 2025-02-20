cls();

for (col = 0; col <= 80; col++)
{
    pos(1, col);

    if (col == 1 || col == 80)
    {
        print("+");
    }
    else
    {
        print("=");
    }
}

pos(2, 3);
print(" Main Menu ");

for (col = 0; col <= 80; col++)
{
    pos(3, col);

    if (col == 1 || col == 80)
    {
        print("+");
    }
    else
    {
        print("=");
    }
}

for (row = 2; row <= 24; row++)
{
    pos(row, 1);
    print("|");
    pos(row, 80);
    print("|");
}

for (col = 0; col <= 80; col++)
{
    pos(24, col);

    if (col == 1 || col == 80)
    {
        print("+");
    }
    else
    {
        print("=");
    }
}

pos(22, 5);
selection = input("Please enter your selection -> ");

if (selection == "q" || selection == "Q")
{
    return true;
}
