<?php

class test {
    function __set($name, &$val) { }
}

$t = new test;
$t->prop = 1;

echo "Done\n";
?>