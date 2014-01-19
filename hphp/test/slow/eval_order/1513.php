<?php

$v = 1;
function foo($a, $b, $c) {
  var_dump($a, $b, $c);
}
function bar($a) {
  foo($a, $a++, $a);
  $arr = array($a, $a++, $a);
  var_dump($arr);
}
bar($v);
