<?php

function test($x) {
  $a = $x;
  $b = $a;
  $a[0]->foo = 1;
  var_dump($a, $b);
  $a = $x;
  $b = $a;
  $a[0][1] = 1;
  var_dump($a, $b);
  $a = $x;
  $c = &$a[0];
  $b = $a;
  $a[0][1] = 1;
  var_dump($a, $b);
  }
test(array(false));
var_dump(array(false));
