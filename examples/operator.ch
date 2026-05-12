// operator.ch — Operator overloading demo
// When we write a + b, it actually computes a^2 + b

polamanna

Op(+) {
    operator := LHS ^ 2 + RHS
}

main() {
    int a;
    a = 3;
    int b;
    b = 5;

    // With the override: a + b  =>  a^2 + b  =>  9 + 5  =>  14
    int result;
    result = a + b;

    "a = " + a sollu
    "b = " + b sollu
    "a + b (with override: LHS^2 + RHS) = " + result sollu

    // Another example
    int c;
    c = 4;
    int d;
    d = 10;
    int result2;
    result2 = c + d;
    "c + d (4^2 + 10) = " + result2 sollu
}

niruthuanna
