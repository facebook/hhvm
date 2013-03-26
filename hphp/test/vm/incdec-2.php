<?php

function test($a, $b) {
  $a++;
  var_dump($a,$b);
  }

$a[] = 1;
test(false, $a);
test(true, $a);
test(1, $a);
test(1.0, $a);

