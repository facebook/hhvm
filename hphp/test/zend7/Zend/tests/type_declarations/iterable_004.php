<?php

class Foo {
    function testArray(array $array) {}

    function testTraversable(Traversable $traversable) {}

    function testScalar(int $int) {}
}

class Bar extends Foo {
    function testArray(iterable $iterable) {}

    function testTraversable(iterable $iterable) {}

    function testScalar(iterable $iterable) {}
}

?>
