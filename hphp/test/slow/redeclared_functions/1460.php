<?php

function test($a, $b, $c, $d, $e, $f, $g = 0) {
  return $a;
}
if (0) {
 function test($a) {
}
 }
test(1,2,3,4,5,6);
