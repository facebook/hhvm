<?php

function f(array $a = null, $e) {
  if (is_int($e) || is_string($e)) {
    $a[$e] ??= array();
    $a[$e]['foo'] = 30;
    $x = $a[$e]['baz'];
    $a[$e]['bar'] = 50;
  } else {
    $a[$e] = 30;
    $x = $a[$e];
    $a[$e] = 50;
  }
  var_dump($a, $x);
}
function g($x, $y) {
  $x[$y]['foo'] = 30;
  $x[$y]['bar'] = 30;
  $z = $x[$y]['bar'];
  $x[$y]['baz'] = 30;
  if ($z) {
    $x[$y]['baz'] = 30;
  }
  return $x;
}
function h($x, $y) {
  if ($x) {
    $x[$y]['foo'] = 30;
    $x[$y]['bar'] = 30;
  }
  var_dump($x[$y]['foo']);
}

<<__EntryPoint>>
function main_534() {
f(array(), 'e');
f(array('e' => array('baz' => 40)), 'e');
var_dump(f(array('y' => array()), 'y'));
var_dump(f(array(), 'y'));
var_dump(f(array(), array()));
h(array(), 0);
h(array(array()), 0);
}
