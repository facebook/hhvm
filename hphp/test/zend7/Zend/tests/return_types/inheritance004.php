<?php
class Foo {
    public static function test() : self {
        return new Foo;
    }
}

class Bar extends Foo {
    public static function test() : parent {
        return new Bar;
    }
}

var_dump(Bar::test());
var_dump(Foo::test());

