<?php

function foo(&$a) {
 var_dump($a++);
 }
function test() {
  foo($a = 6);
  $a = null;
  foo($b += 5);
  $b = null;
  foo($c -= 5);
  $c = null;
  $e = 0;
  foo(++$e);
  $e = 5;
  $g = 0;
  foo(--$g);
  $g = 7;
  $h = null;
  foo($h += 5);
  $h = null;
  foo($h -= 5);
  $h = null;
}
test();
