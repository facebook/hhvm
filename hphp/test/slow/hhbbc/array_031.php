<?php

function junk() { return 2; }
function bar() {
  $y = null;
  $x = array('z' => junk());
  $y =& $x['z'];
  $y = 'heh';
  $val = $x['z'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_array($x));
  var_dump($x);
}

<<__EntryPoint>>
function main_array_031() {
bar();
}
