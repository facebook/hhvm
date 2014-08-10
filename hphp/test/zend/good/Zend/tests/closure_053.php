<?php

class A {
    function foo() {
        $f = static function() {
            return self::class;
        };
        return $f();
    }
}

class B extends A {}

$b = new B;
var_dump($b->foo());

