list = "Blake Chelsea Lucy Isaac";

if (list_contains(list, "Blake"))
{
    println("Contains Blake");
}

if (list_contains(list, "Lucy"))
{
    println("Contains Lucy");
}

if (!list_contains(list, "Skippy"))
{
    println("Does not contain Skippy");
}

list = list_add(list, "Skippy");

if (!list_contains(list, "Skippy"))
{
    println("Does not contain Skippy");
}
else
{
    println("DOES now contain Skippy");
}

list = list_remove(list, "Skippy");

if (list_contains(list, "Blake") && !list_contains(list, "Pell"))
{
    list = list_add(list, "Pell");
}

println("The final list is: ${list}");

return true;