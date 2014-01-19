<?php

function foo() {
  yield 1 => 2;
  yield "a" => "b";
}

$gen = foo();
$gen->next();
var_dump($gen->key());
var_dump($gen->current());
$gen->next();
var_dump($gen->key());
var_dump($gen->current());
