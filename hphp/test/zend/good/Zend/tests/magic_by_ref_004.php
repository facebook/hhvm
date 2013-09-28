<?php

class test {
    function __unset(&$name) { }
}

$t = new test;
$name = "prop";

var_dump($t->$name);

echo "Done\n";
?>