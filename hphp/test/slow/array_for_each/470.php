<?php

function g3() {
  $arr2 = array(0,1,2,3);
  $arr = $arr2;
  $b = true;
  $b2 = true;
  foreach ($arr as &$v) {
    yield null;
    echo "val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      $arr = array(4,5,6,7);
    }
 else if ($b2 && $v == 6) {
      $b2 = false;
      $arr = $arr2;
    }
  }
}
foreach (g3() as $_) {
}
