<?php

class test {
    function __isset(&$name) { }
}
<<__EntryPoint>> function main() {
$t = new test;
$name = "prop";

var_dump(isset($t->$name));

echo "Done\n";
}
