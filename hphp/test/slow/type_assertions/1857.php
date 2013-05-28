<?php

function f($x) {
  while (is_array($x) && isset($x[0])) $x = $x[0];
  var_dump($x);
}
function g($x) {
  for (;
 is_array($x) && isset($x[0]);
 $x = $x[0]);
  var_dump($x);
}
function h($x) {
  if (!is_array($x) || !isset($x[0])) return;
  do {
    $x = $x[0];
  }
 while (is_array($x) && isset($x[0]));
  var_dump($x);
}
f(array(array(array(array('hello')))));
g(array(array(array(array('hello')))));
h(array(array(array(array('hello')))));
