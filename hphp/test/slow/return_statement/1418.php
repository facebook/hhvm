<?php

function foo1($a) {
  if ($a) return "ok";
}
function foo2($a) {
  if ($a > 1) return;
  if ($a == 1) return 1;
}
function foo3($a) {
  if ($a > 1) return;
  if ($a == 1) return;
}
function bar() {
  $v1 = foo1(0);
  var_dump($v1);
  $v2 = foo2(0);
  var_dump($v2);
  $v3 = foo3(0);
  var_dump($v3);
}
bar();
