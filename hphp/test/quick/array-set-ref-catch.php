<?php

function test(&$a, $b) {
  for ($i = $j = 0; $i < 40000; $i++, $j += 0x10000) {
    $a[$j] = $b;
  }
  var_dump($a);
}

$y = null;
test($y, 5);
