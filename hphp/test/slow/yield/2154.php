<?php

function f() {
  $a = function() {
 yield 1;
 yield 2;
 }
;
  return $a;
}
$f = f();
foreach ($f() as $v) {
 var_dump($v);
 }
