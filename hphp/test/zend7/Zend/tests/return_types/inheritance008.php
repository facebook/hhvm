<?php
require __DIR__ . "/classes.php.inc";

class Foo {
    public static function test() : A {
        return new A;
    }
}

class Bar extends Foo {
    public static function test() : A {
        return new B;
    }
}

var_dump(Bar::test());
var_dump(Foo::test());

