<?php

function junk() { return 2; }
function bar() {
  $x = array('y' => junk());
  $y =& $x['x']->hehe;
  $val = $x['x'];
  var_dump(is_object($y));
  var_dump(is_null($y));
  var_dump(is_null($val));
  var_dump(is_array($x));
}
bar();
