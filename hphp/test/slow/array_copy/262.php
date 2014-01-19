<?php

function f($b) {
  $a = $b ? 0 : array($b);
  $a[1][0] = $a;
 var_dump($a);
}
f(false);
