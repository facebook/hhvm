<?php

function foo() { mt_rand(); mt_rand(); mt_rand(); return new stdclass(); }

function bar(&$k, &$z) {
  $y = foo();
  echo $z;
}


<<__EntryPoint>>
function main_refcount002() {
  $k = "asd";
  bar(&$k, &$k);
}
