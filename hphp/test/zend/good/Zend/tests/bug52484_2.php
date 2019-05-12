<?php

class A {
    function __set($prop, $val) {
        $this->$prop = $val;
    }
}
<<__EntryPoint>> function main() {
$a = new A();
$prop = null;

$a->$prop = 2;
}
