<?php

class A {
    function foo() {
        $f = function() {
            return static::class;
        };
        return $f();
    }
}

class B extends A {}

$b = new B;
var_dump($b->foo());

