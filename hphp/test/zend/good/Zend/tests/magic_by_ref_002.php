<?php

class test {
    function __set($name, &$val) { }
}
<<__EntryPoint>> function main() {
$t = new test;
$t->prop = 1;

echo "Done\n";
}
