<?php
class foo {
    public function bar() : callable {
        $test = "one";
        return function() use($test) : array {
            return null;
        };
    }
}

$baz = new foo();
var_dump($func=$baz->bar(), $func());

