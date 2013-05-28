<?php

function f(array $a = null, $e) {
  $a[$e]['foo'] = 30;
  $x = $a[$e]['baz'];
  $a[$e]['bar'] = 50;
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
f(null, 'e');
f(array(), 'e');
f(array('e' => array('baz' => 40)), 'e');
var_dump(f(array('y' => array()), 'y'));
var_dump(f(array(), 'y'));
var_dump(f(array(), array()));
h(array(), 0);
h(array(array()), 0);
