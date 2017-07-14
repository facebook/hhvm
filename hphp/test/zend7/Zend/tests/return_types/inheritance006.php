<?php
require __DIR__ . "/classes.php.inc";

class Foo {
    public static function test() : A {
        return new A;
    }
}

class Bar extends Foo {
    public static function test() : B {
        return new B;
    }
}

