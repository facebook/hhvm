<?php
class Foo {
    public static function test() : Traversable {
        return new ArrayIterator([1, 2]);
    }
}

class Bar extends Foo {
    public static function test() : Traversable {
        return new ArrayObject([1, 2]);
    }
}

var_dump(Bar::test());
var_dump(Foo::test());

