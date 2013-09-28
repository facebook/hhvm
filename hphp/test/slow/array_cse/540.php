<?php

function foo($a, $b, $c, $d) {
  var_dump($a, $b, $c, $d);
}
function bar(&$a, $b, $c, $d) {
  var_dump($a, $b, $c, $d);
}
function f($a) {
  foo($a[0], $a[0], $a[0], $a[0]++);
  foo($a[0], $a[0], $a[0], $a[0]);
}
function g($a) {
  bar($a[0], $a[0], $a[0], $a[0]++);
  bar($a[0], $a[0], $a[0], $a[0]);
}
f(array(0));
g(array(0));
