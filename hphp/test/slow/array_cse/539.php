<?php

function id($x) {
  return $x;
}
function f1($x) {
  $z = id($x[0]);
  foreach ($x[0] as $a) {
    $z[] = array(id($z), count($x[0]));
  }
}
f1(array(array(0, 1, 2, 3)));

function f2($x) {
  var_dump($x[0]);
  $y = 'foo' . $x[0] . 'bar';
}
f2('foobar');

function f3($x) {
  $x = is_string($x[0]) ? $x[0] : get_class($x[0]);
  return $x;
}
var_dump(f3('abc'));
var_dump(f3(array(new stdClass)));
