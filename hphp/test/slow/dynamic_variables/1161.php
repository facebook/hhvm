<?php

function foo() {
  static $b = 20;
  global $d;
  $a = 10;
  $b = 'c';
  $$b = 20;
  $gdv = get_defined_vars();
  var_dump(isset($gdv['a']) && $gdv['a'] === 10);
  var_dump(isset($gdv['b']) && $gdv['b'] === 'c');
  var_dump(isset($gdv['c']) && $gdv['c'] === 20);
  var_dump(isset($gdv['d']) && $gdv['d'] === 2.1);
}
$d = 2.1;
foo();
var_dump(isset($ggdv['argc']));
var_dump(isset($ggdv['argv']));
var_dump(isset($ggdv['_SERVER']));
var_dump(isset($ggdv['_GET']));
var_dump(isset($ggdv['_POST']));
var_dump(isset($ggdv['_COOKIE']));
var_dump(isset($ggdv['_FILES']));
var_dump(isset($ggdv['d']));
