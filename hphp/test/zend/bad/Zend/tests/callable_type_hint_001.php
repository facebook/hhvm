<?php

class bar {
    function baz() {}
    static function foo() {}
}
function foo(callable $bar) {
    var_dump($bar);
}
$closure = function () {};

foo("strpos");
foo("foo");
foo(array("bar", "baz"));
foo(array("bar", "foo"));
foo($closure);