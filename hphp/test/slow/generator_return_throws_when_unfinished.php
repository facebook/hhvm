<?php

function gen() {
  yield 1; yield 2; yield 3;
  return 11;
}

$g = gen();

$g->next();

var_dump($g->getReturn());
