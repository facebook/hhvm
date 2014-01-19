<?php

$a = 1;
function test() {
  $b = 1;
  global $a;
  $a = 10;
}
var_dump($a);
test();
var_dump($a);
  return true;
