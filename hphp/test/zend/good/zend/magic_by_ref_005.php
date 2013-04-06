<?php

class test {
    function __isset(&$name) { }
}

$t = new test;
$name = "prop";

var_dump(isset($t->$name));

echo "Done\n";
?>