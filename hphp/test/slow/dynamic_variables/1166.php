<?php

$MY_VAR_a = 123;
function foo() {
  global $MY_VAR_a;
  $arr = get_defined_vars();
  asort($arr);
  var_dump($arr);
  return $arr;
}
foo();
