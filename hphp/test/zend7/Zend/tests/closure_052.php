<?php

class A {
    static function foo() {
        $f = function() {
            return static::class;
        };
        return $f();
    }
}

class B extends A {}

var_dump(B::foo());

