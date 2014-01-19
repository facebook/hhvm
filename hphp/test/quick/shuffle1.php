<?php

function get($a, $idx) {
  $max = 10;
  for ($i = 0; $i < $max; $i++) {
    $r = $a[$idx];
    $r = $a[0];
    $r = $a[1];
    $r = $a[2];
  }
  return $a[$idx];
}

$a = array('a', 2, false);
var_dump(get($a, 1));

$a = array('a'=>'b', 1=>'c', 2=>4, 'c'=>3, 0=>'hello');
var_dump(get($a, 1));

