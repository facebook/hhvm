<?php

function a($a,$b) {
  var_dump(xdebug_get_declared_vars());
}
a(52, 52);

function b($a,$b) {
  echo $a;
  echo $b, "\n";
  var_dump(xdebug_get_declared_vars());
}
b(52, 52);

function c($a,$b) {
  echo $a;
  echo $b, "\n";
  unset($b);
  var_dump(xdebug_get_declared_vars());
}
c(3.14, 159);

function d($a,$b) {
  $c = 3;
  $d = 4;
  echo $a, "\n";
  var_dump(xdebug_get_declared_vars());
}
d(1, 2);

function s()
{
  $c = 42;
  $d = 54;
  echo $c, $d, "\n";
  var_dump(xdebug_get_declared_vars());
}

register_shutdown_function('s');
