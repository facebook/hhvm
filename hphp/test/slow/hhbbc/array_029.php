<?php

function junk() { return 2; }
function bar() {
  $y = null;
  $x = array('z' => junk());
  $x['x']['y'] =& $y;
  $val = $x['x'];
  $val2 = $x['x']['y'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_null($val2));
  var_dump(is_array($val2));
  var_dump(is_array($x));
  var_dump($x);
}
bar();
