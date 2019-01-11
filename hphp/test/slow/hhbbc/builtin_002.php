<?php

function fn_name() { return 'array_multisort'; }

function foo() {
  $fn = fn_name();

  $x = array(2, 3, 12, 0, 52, 4);
  $y = array("a", "b", "c", "d", "e", "f");
  $fn(&$x, &$y);
  var_dump($x);
  var_dump($y);
  $x = array(2, 3, 12, 0, 52, 4);
  $y = array("a", "b", "c", "d", "e", "f");
  $desc = SORT_DESC;
  $fn(&$y, &$desc, &$x);
  var_dump($x);
  var_dump($y);
}


<<__EntryPoint>>
function main_builtin_002() {
foo();
}
