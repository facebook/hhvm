<?hh

class A {
    <<__LSB>> private static string $x;
}

class B extends A {
    <<__LSB>> public static string $x;
}

