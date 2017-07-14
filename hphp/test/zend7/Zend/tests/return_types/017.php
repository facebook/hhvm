<?php

namespace FooSpace;

trait Fooable {
    function foo(): \Iterator {
        return new \EmptyIterator();
    }
}

class Foo {
    use Fooable;
}

$foo = new Foo;
var_dump($foo->foo([]));

