<?php

function h5() {
  $x = array(1,2,3,4);
  end($x);
  next($x);
  $y = $x;
  $y[] = 4;
  var_dump(current($x));
  var_dump(current($y));
}
h5();
