<?hh

class A {
    <<__LSB>> private static ?string $x = null;

    static function foo($v) :mixed{
        if (static::$x === null) static::$x = $v;
        var_dump(static::$x);
    }
}

class B extends A {
    static function bar() :mixed{
        var_dump(static::$x);
    }
}

class C extends B {
}
<<__EntryPoint>> function main(): void {
A::foo("A");
B::foo("B");
C::foo("C");

A::foo("");
B::foo("");
C::foo("");

B::bar();
}
