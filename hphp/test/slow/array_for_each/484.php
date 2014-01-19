<?php

function g10() {
  $arr = array(0,1,2,3);
  $b = true;
  foreach ($arr as &$v) {
    yield null;
    echo "val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      array_unshift($arr, 4);
    }
  }
}
foreach (g10() as $_) {
}
