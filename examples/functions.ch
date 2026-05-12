// functions.ch — Function calls demo
// Demonstrates: user-defined functions with parameters

polamanna

add(int a, int b):
    int result;
    result = a + b;
    "  add(" + a + ", " + b + ") = " + result sollu


factorial_iter(int n) {
    int result;
    result = 1;
    int i;
    i = 1;
    while (i <= n) {
        result = result * i;
        i = i + 1;
    }
    "  factorial(" + n + ") = " + result sollu
}

greet(string name) {
    "  Hello, " + name + "!" sollu
}

main() {
    "=== Function Calls ===" sollu
    add(3, 7);
    add(100, 200);

    "=== Factorial ===" sollu
    factorial_iter(5);
    factorial_iter(10);

    "=== Greetings ===" sollu
    greet("Chennai");
    greet("World");
}

niruthuanna
