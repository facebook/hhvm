<?php

function gen() {
  echo "yielding\n";
  yield 24;
  echo "returning\n";
  return 42;
}

$g = gen();
$g->next();
var_dump($g->current());
$g->next();
var_dump($g->current());
