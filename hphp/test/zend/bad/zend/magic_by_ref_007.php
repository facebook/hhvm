<?php

class test {
    function __call($name, &$args) { }
}

$t = new test;
$func = "foo";
$arg = 1;

$t->$func($arg);

echo "Done\n";
?>