<?php

function h1() {
  $x = array(1,2,3,4);
  next($x);
  $y = $x;
  unset($y[2]);
  var_dump(current($x));
  var_dump(current($y));
}
h1();
