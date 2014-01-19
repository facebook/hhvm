<?php

function f() {
  var_dump(func_get_args());
}
function g($x) {
  if ($x) $f = 'f';
  else    $f = '__nocall__';
  call_user_func_array($f,     array('x' => 10, 'y' => 20, 'z' => 30, 'j' => 40));
  call_user_func_array($f,     array(3 => 10, 80 => 20, 10 => 30, 30 => 40));
}
g(10);
