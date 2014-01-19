<?php

function blocker() {
 print 'block';
 }
function id($x) {
 return $x;
 }
function f($x, $y) {
  $y = $x[$y[0]] ? $x[$y[0]] : id($x[$y[0]]);
  blocker();
  var_dump($y);
}
