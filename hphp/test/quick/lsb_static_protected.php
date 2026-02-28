<?hh

class A {
    <<__LSB>> protected static ?string $x = null;

    static function init() :mixed{
        if (static::$x === null) static::$x = static::class;
    }

    static function afunc() :mixed{
        static::init();
        var_dump(self::$x . " " . static::$x);
    }

    static function apeek() :mixed{
        var_dump(A::$x . " " . B::$x);
    }
}

class B extends A {
    static function bfunc() :mixed{
        static::init();
        var_dump(self::$x . " " . static::$x);
    }

    static function bpeek() :mixed{
        var_dump(A::$x . " " . B::$x);
    }
}
<<__EntryPoint>> function main(): void {
A::afunc();
B::afunc();
B::bfunc();

A::apeek();
B::bpeek();

var_dump(B::$x);
}
