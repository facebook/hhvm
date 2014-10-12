<?php
const ONE = 1;
const TWO = 3.1415;
const THREE = "this is a constant string";
const FOUR = true;

function params($a = "string param", $b = ONE, $c=TWO,
		            $d=THREE, $e=FOUR, $f=[1,2,3]) {
}

function test($param) {
  $r = new ReflectionParameter('params', $param);
  var_dump($r->isDefaultValueConstant());
}

test('a');
test('b');
test('c');
test('d');
test('e');
test('f');
