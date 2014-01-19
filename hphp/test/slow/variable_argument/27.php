<?php

function f() {
  var_dump(func_get_arg(-1));
  var_dump(func_get_arg(0));
  var_dump(func_get_arg(1));
  if (func_get_arg(2)) {
    $x = 0;
  }
 else {
    $x = 1;
  }
  var_dump(func_get_arg($x++));
}
function g($x, &$y) {
  var_dump(func_get_arg(-1));
  var_dump(func_get_arg(0));
  var_dump(func_get_arg(1));
  var_dump(func_get_arg(2));
  var_dump(func_get_arg(3));
}
function h($x, &$y, array $z) {
  var_dump(func_get_arg(-1));
  var_dump(func_get_arg(0));
  var_dump(func_get_arg(1));
  var_dump(func_get_arg(2));
  var_dump(func_get_arg(3));
  var_dump(func_get_arg(4));
}
function i(&$x) {
  $x = 30;
  var_dump(func_get_args());
  var_dump(func_get_arg(0));
  $y =& func_get_arg(0);
  $y = 40;
  var_dump(func_get_arg(0));
}
f(10);
$x = 1;
g(0, $x, 2);
h(0, $x, array(1, 2), 3);
$x = 10;
i($x);
i();
