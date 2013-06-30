<?php

function foo() {
  yield "abc" => "def";
}

$x = foo();
$x->next();
$y = clone $x;
var_dump($x->key() === $y->key());
var_dump($x->current() === $y->current());
