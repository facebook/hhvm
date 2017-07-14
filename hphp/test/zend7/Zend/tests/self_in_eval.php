<?php

class C {
    const FOO = 1;
    private static $bar = 2;
    public static function f() {
        eval(<<<'PHP'
var_dump(self::FOO);
var_dump(self::$bar);
var_dump(self::class);
var_dump(static::class);
PHP
        );
    }
}

C::f();

?>
