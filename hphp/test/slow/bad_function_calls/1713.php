<?php

function f() {
  $a = array();
  $a[] = 1;
  array_push(&$a);
  var_dump($a);
}

<<__EntryPoint>>
function main_1713() {
f();
}
