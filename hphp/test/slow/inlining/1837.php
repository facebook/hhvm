<?php

/* Compile only: verify no c++ compilation errors */function foo($a) {
  return $a[1];
}
function baz(&$x) {
 if ($x) $x++;
 }
function bar($a) {
  baz(foo($a)[1]);
  foo($a)->bar = 1;
}
