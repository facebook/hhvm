<?php

class Foo {
    function a(NonExisting $foo) {}
}
<<__EntryPoint>> function main() {
$o = new Foo;
$o->a($o);
}
