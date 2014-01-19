<?php

function iss($a, $i) {
  $max = 10;
  for ($i = 0; $i < $max; $i++) {
    isset($a[$i]);
    isset($a[0]);
    isset($a[1]);
    isset($a[2]);
  }
  return isset($a[$i]);
}

$a = array('a', 2, false);
var_dump(iss($a, 5));

$a = array('a'=>'b', 1=>'c', 2=>4, 'c'=>3, 0=>'hello');
var_dump(iss($a, 5));

