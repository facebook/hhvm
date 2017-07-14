<?php

namespace Collections;

class Foo {
    function foo(\Iterator $i): \Iterator {
        return $i;
    }
}

$foo = new Foo;
var_dump($foo->foo(new \EmptyIterator()));

