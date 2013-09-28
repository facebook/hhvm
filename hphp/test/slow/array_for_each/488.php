<?php

function g12($a) {
  var_dump($a);
  $arr = array(0,1,2,3);
  $b = true;
  foreach ($arr as &$v) {
    yield null;
    if ($b && $v == 1) {
      $b = false;
      $arr = $a;
    }
 else {
      $v = 5;
    }
  }
  var_dump($a);
}
foreach (g12(array(0, 0, 0, 0)) as $_) {
}
