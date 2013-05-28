<?php

function f1($x) {
  if ($x[1]) goto x_t;
  var_dump($x[0]);
  var_dump($x[0]);
  x_t:
  var_dump($x[0]);
}
function f2($x) {
  $i = 3;
  while ($x[0] && $i--) {
    var_dump($x[0]);
  }
}
function f3($x, $y) {
  do {
    var_dump($x[0]);
  }
 while ($x[0] && $y);
  var_dump($x[0]);
}
function f4($x) {
  foreach ($x[0] as $k => $v) {
    var_dump($x[0]);
    var_dump($k, $v);
  }
}
function f5($x) {
  switch ($x[0]) {
  case 0:
    var_dump($x[0]);
  }
}
function f6($x, $y, $z) {
  if ($z) goto my_clause;
  if ($y) {
 var_dump($y);
 }
  else if ($x[0]) {
    var_dump($x[0]);
    my_clause:
    var_dump($x);
  }
}
f1(array(0, 0));
f2(array(10));
f3(array(10), false);
f4(array(array(1, 2, 3)));
f5(array(false, false));
f6(array(true), false, false);
