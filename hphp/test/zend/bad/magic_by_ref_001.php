<?php

class test {
    function __set(&$name, $val) { }
}

$t = new test;
$name = "prop";
$t->$name = 1;

echo "Done\n";
?>