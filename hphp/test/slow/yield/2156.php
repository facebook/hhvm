<?php

function f() {
  $a = yield 1;
 list($a, $b) = yield $a;
 yield $b;
}
$c = f();
$c->next();
var_dump($c->current());
$c->send(2);
var_dump($c->current());
$c->send(array(3, 4));
var_dump($c->current());
