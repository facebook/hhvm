<?php
class A {
    public static function f() {
        return function () {
            var_dump(self::class);
            var_dump(static::class);
        };
    }
}

class B extends A {}

$f = B::f();
$f();

$g = $f->bindTo(null, A::class);
$g();

$foo = function () {
    var_dump(self::class);
    var_dump(static::class);
};

$bar = $foo->bindTo(null, A::class);
$bar();

?>
