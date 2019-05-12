<?php

class test {
    function __unset(&$name) { }
}
<<__EntryPoint>> function main() {
$t = new test;
$name = "prop";

var_dump($t->$name);

echo "Done\n";
}
