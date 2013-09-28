<?php

function foo(&$x, $y) {
 $x++;
 var_dump($x);
 }
function bar(&$x, $y, $f) {
  $f($x, $x = &$y);
  foo($x, $x = &$y);
  foo($y, $y = 2);
}
$x = 0;
bar($x, $x, 'foo');
var_dump($x);
