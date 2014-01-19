<?php

class test {
    function __call(&$name, $args) { }
}

$t = new test;
$func = "foo";

$t->$func();

echo "Done\n";
?>