result = true;
list = "Blake Chelsea Lucy Isaac";

if (!list_contains(list, "Blake"))
{
    result = false;
}

if (!list_contains(list, "Lucy"))
{
    result = false;
}

if (list_contains(list, "Skippy"))
{
    result = false;
}

list = list_add(list, "Skippy");

if (!list_contains(list, "Skippy"))
{
    result = false;
}

list = list_remove(list, "Skippy");

if (list_contains(list, "Blake") && !list_contains(list, "Pell"))
{
    list = list_add(list, "Pell");
}

if (list_contains(list, "Skippy"))
{
    result = false;
}

if (result)
{
    println("[ SUCCESS ] :: lists.arc");
}
else
{
    println("[ FAILED ] :: lists.arc");
}

return result;