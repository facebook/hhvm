<?php

function squares_cubes() {
  $i = 0;
  for (;;) {
    $i++;
    yield $i*$i => $i*$i*$i;
  }
}

$c = squares_cubes();
$c->next();
var_dump($c->key());
var_dump($c->current());
$c->next();
var_dump($c->key());
var_dump($c->current());
