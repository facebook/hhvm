<?php

function g1() {
  yield 2;
  yield 3;
  return 42;
}

function g2() {
  yield 1;
  $g1result = yield from g1();
  yield 4;
  return $g1result;
}

$g = g2();
foreach ($g as $yielded) {
  var_dump($yielded);
}
var_dump($g->getReturn());
