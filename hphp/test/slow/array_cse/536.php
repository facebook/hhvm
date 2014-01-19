<?php

function f($x) {
  var_dump($x[0]);
  $x =& $x[0];
  $x[0] = 30;
  var_dump($x[0]);
}
function g($x) {
  $x[0][1] = 10;
  var_dump($x[0][2]);
}
f(array(0));
g(array());
g(array(0));
