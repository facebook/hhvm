<?php

function g9() {
  $arr = array(0,1,2,3,4);
  $b = true;
  foreach ($arr as &$v) {
    yield null;
    echo "val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      array_shift($arr);
    }
  }
}
foreach (g9() as $_) {
}
