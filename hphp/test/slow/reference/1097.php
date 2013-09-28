<?php

function foo($x) {
  var_dump($x[0]->foo['a']['b']);
  $x[0]->foo['a']['b'] = 5;
  var_dump($x);
}
foo(false);
function baz(&$x) {
}
foreach ($x->foo[1]->prop as &$y) {
}
var_dump($x);
baz($q->foo[1]->prop);
var_dump($q);
$y = &$z->foo[1]->prop;
var_dump($z);
function &fiz(&$x) {
  return $x->foo[1]->prop;
}
fiz($w);
var_dump($w);
