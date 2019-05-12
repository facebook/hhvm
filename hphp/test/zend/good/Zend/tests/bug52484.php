<?php

class A {
    function __unset($prop) {
        unset($this->$prop);
    }
}
<<__EntryPoint>> function main() {
$a = new A();
$prop = null;

unset($a->$prop);
}
