<?php

function foo() {
  $arr = get_defined_vars();
  var_dump($arr);
  return $arr;
}
foo();
