<?php

function junk() { return 2; }
function bar() {
  $y = null;
  $x = array('l' => junk());
  $y =& $x['z']['k'];
  $y = 'heh';
  $val = $x['z'];
  $val2 = $x['z']['k'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_null($val2));
  var_dump(is_array($val2));
  var_dump(is_array($x));
  var_dump($x);
}
bar();
