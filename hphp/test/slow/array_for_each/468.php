<?php

function g2() {
  $arr = array(0,1,2,3);
  $b = true;
  foreach ($arr as &$v) {
    yield null;
    echo "val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      $old = $arr;
      $arr = array(4,5,6,7);
    }
 else if ($v == 6) {
      $arr = $old;
      unset($old);
    }
  }
}
foreach (g2() as $_) {
}
