<?php

function a() { return 4; }
function junk() { return array('z' => a()); }
function bar() {
  $y = null;
  $x = array('z' => junk());
  unset($x['z']['z']);
  $val = $x['z'];
  $val1 = $x['z']['z'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_null($val1));
  var_dump(is_int($val1));
  var_dump(is_array($x));
  var_dump($x);
}
bar();
