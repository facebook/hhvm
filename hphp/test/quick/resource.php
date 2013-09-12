<?php

class Foo{}

function f(resource $r) {
}

$r = STDIN;
var_dump($r);
var_dump($r instanceof resource);
f($r);

$foo = new Foo();
var_dump($foo);
var_dump($foo instanceof resource);
f($foo);
