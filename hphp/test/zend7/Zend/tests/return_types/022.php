<?php
$foo = "bar";
$test = function() use($foo) : Closure {
    return function() use ($foo) {
        return $foo;
    };
};

$callable = $test();
var_dump($callable());

