<?php

function h2() {
  $x = array(1,2,3,4);
  next($x);
  $y = $x;
  $y[] = 4;
  var_dump(current($x));
  var_dump(current($y));
}
h2();
