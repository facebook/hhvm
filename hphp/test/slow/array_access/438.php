<?php

function test($x) {
  $a = $x;
  $b = $a;
  $a[0]->foo = 1;
  var_dump($a, $b);
  $a = $x;
  $b = $a;
  $a[0] = array();
  $a[0][1] = 1;
  var_dump($a, $b);
  }

<<__EntryPoint>>
function main_438() {
test(array(false));
var_dump(array(false));
}
