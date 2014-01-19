<?php

function h6() {
  $x = array(1,2,3,4);
  end($x);
  next($x);
  $y = $x;
  array_pop($y);
  var_dump(current($x));
  var_dump(current($y));
}
h6();
