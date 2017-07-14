<?php
class Foo {
    public static function test() : Traversable {
        return new ArrayIterator([1, 2]);
    }
}

class Bar extends Foo {
    public static function test() : ArrayObject {
        return new ArrayObject([1, 2]);
    }
}

