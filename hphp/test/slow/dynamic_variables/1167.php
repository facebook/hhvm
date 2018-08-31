<?php

function foo() {
  $arr = get_defined_vars();
  var_dump($arr);
  return $arr;
}

<<__EntryPoint>>
function main_1167() {
foo();
}
