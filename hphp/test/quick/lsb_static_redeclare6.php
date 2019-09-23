<?hh

class A {
    <<__LSB>> public static string $x;
}

class B extends A {
}

class C extends B {
    public string $x;
}

