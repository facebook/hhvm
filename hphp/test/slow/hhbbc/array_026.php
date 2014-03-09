<?php

function junk() { return 2; }
function bar() {
  $x = array('x' => junk());
  $x['x'] += 1;
  $val = $x['x'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_array($x));
  var_dump($x);
}
bar();
