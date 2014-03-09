<?php

function junk() { return 2; }
function bar() {
  $x = array('y' => junk());
  $x['x']['z'] += 1;
  $val = $x['x'];
  $val2 = $x['x']['z'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_null($val2));
  var_dump(is_array($val2));
}
bar();
