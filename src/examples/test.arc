var name = "  Blake Pell    "
name = trim(name)
println(`Hello ${name}`)

name = trim("Blake Pell")
println(name)

name = trim("   Blake Pell")
println(name)

print(left(name, 5))
print(" ")
println(right(name, 4))

name = trim("")
println(name)

name = trim("     ")
println(name)