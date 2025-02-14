first_name = input("Enter your first name: ");
last_name = input("Enter your last name: ");

if (first_name == "" || last_name == "")
{
    println("You must enter both a first and last name.");
    return false;
}

println("Hello ${first_name} ${last_name}");
return true;