<?php

class A {
    static function foo() {
        $f = function() {
            return self::class;
        };
        return $f();
    }
}

class B extends A {}

var_dump(B::foo());

