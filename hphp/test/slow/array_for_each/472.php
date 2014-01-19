<?php

function g4() {
  $arr = array(0,1,2,3);
  $b = true;
  foreach ($arr as &$v) {
    yield null;
    echo "val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      array_push($arr, 4);
    }
  }
}
foreach (g4() as $_) {
}
