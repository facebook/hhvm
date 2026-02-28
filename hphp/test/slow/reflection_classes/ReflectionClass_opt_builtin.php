<?hh

trait T {}

interface I {}

final class F {}

abstract class C {
    public static function baz(): void{
        var_dump((new ReflectionClass(static::class))->isAbstract());
        var_dump((new ReflectionClass(static::class))->getName());
    }
}

class B extends C {
    public static function foo(): void{
        var_dump((new ReflectionClass(self::class))->isAbstract());
        var_dump((new ReflectionClass(self::class))->getName());
    }

    public static function bar(): void{
        var_dump((new ReflectionClass(parent::class))->isAbstract());
        var_dump((new ReflectionClass(parent::class))->getName());
    }
}

<<__EntryPoint>>
function main() :mixed{
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
    // C
    var_dump((new ReflectionClass(C::class))->getName());
    var_dump((new ReflectionClass('C'))->getName());
    // T
    var_dump((new ReflectionClass('T'))->getName());
    var_dump((new ReflectionClass(T::class))->getName());
    // I
    var_dump((new ReflectionClass(I::class))->getName());
    // B
    var_dump((new ReflectionClass(new B))->getName());
    var_dump((new ReflectionClass($b))->getName());
    var_dump((new ReflectionClass(B::class))->getName());

    var_dump((new ReflectionClass(F::class))->isFinal());
    var_dump((new ReflectionClass(B::class))->isFinal());

    var_dump((new ReflectionClass(I::class))->isInterface());
    var_dump((new ReflectionClass(B::class))->isInterface());
}
