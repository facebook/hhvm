<?php

function foo($x) {
  var_dump($x[0]->foo['a']['b']);
  $x[0]->foo['a']['b'] = 5;
  var_dump($x);
}
function baz(&$x) {
}

<<__EntryPoint>>
function main_1097() {
  foo(false);
  foreach ($x->foo[1]->prop as &$y) {
  }
  var_dump($x);
  baz($q->foo[1]->prop);
  var_dump($q);
  $y = &$z->foo[1]->prop;
  var_dump($z);
}
