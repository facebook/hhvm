<?php

const ONE = 1;
const TWO = ONE + ONE;

function params($one = ONE, $four = TWO * TWO, $ten = TWO * 5) {
  return $one + $four + $ten;
}

function test($param) {
  $r = new ReflectionParameter('params', $param);

  var_dump($r->getDefaultValue());
  var_dump($r->getDefaultValueText());
  var_dump($r->getDefaultValueConstantName());
}

test('one');
test('four');
test('ten');
