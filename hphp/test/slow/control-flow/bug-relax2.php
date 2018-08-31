<?php

function foo($arr) {
  $children = array();
  foreach ($arr as $child) {
    $children[] = $child;
  }
  return $children;
}


<<__EntryPoint>>
function main_bug_relax2() {
var_dump(foo(array()));
var_dump(foo(array(1)));
var_dump(foo(array(2)));
}
