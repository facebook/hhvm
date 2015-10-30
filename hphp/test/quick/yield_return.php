<?php

function gen() {
  echo "yielding\n";
  yield 24;
  echo "returning\n";
  return 42;
}

$g = gen();
var_dump($g->valid());
var_dump($g->current());
$g->next();
var_dump($g->valid());
var_dump($g->current());
var_dump($g->getReturn());
