<?php

function foo() { mt_rand(); mt_rand(); mt_rand(); return new stdclass(); }

function bar() {
  $k = "asd";
  $z =& $k;
  $y = foo();
  echo $z;
}


<<__EntryPoint>>
function main_refcount002() {
bar();
}
