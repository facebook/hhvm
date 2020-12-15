<?hh

trait T {}

interface I {}

abstract class C {
    public static function baz(): void{
        var_dump((new ReflectionClass(static::class))->isAbstract());
    }
}

class B extends C {
    public static function foo(): void{
        var_dump((new ReflectionClass(self::class))->isAbstract());
    }

    public static function bar(): void{
        var_dump((new ReflectionClass(parent::class))->isAbstract());
    }
}

<<__EntryPoint>>
function main() {
    $b = new B();
    //true
    B::bar();
    C::baz();
    var_dump((new ReflectionClass(C::class))->isAbstract());
    var_dump((new ReflectionClass('C'))->isAbstract());
    //false
    B::foo();
    B::baz();
    var_dump((new ReflectionClass(T::class))->isAbstract());
    var_dump((new ReflectionClass(I::class))->isAbstract());
    var_dump((new ReflectionClass(B::class))->isAbstract());
    var_dump((new ReflectionClass('T'))->isAbstract());
    var_dump((new ReflectionClass('I'))->isAbstract());
    var_dump((new ReflectionClass('B'))->isAbstract());
    var_dump((new ReflectionClass($b))->isAbstract());
    var_dump((new ReflectionClass(new B))->isAbstract());
}
