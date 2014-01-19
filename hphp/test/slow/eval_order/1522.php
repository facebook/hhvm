<?php

function foo($v) {
  $a = array('key' => &$v);
  return $a;
}
function goo($v) {
  return $v . 1;
}
var_dump(foo('1.0'));
var_dump(foo(foo('1.0')));
var_dump(foo(goo('1.0')));
