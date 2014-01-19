<?php

function g1() {
  $arr = array(0,1,2,3);
  $b = true;
  foreach ($arr as &$v) {
    echo "val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      $arr = array(4,5,6,7);
     }
  }
}
g1();
