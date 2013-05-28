<?php

function f() {
  $a = 0;
  $b = 0;
  $c = 0;
  $d = 0;
  array_chunk($a = 1, $b = 2, $c = 3, $d = 4);
  var_dump($a, $b, $c, $d);
}
f();
