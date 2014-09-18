<?php

function foo($arr) {
  $children = array();
  foreach ($arr as $child) {
    $children[] = $child;
  }
  return $children;
}

var_dump(foo(array()));
var_dump(foo(array(1)));
var_dump(foo(array(2)));
