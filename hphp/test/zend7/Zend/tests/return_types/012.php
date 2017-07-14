<?php
class foo {
    public function bar() : callable {
        $test = "one";
        return function() use($test) : array {
            return array($test);
        };
    }
}

$baz = new foo();
var_dump($baz->bar());

