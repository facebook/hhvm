<?php

function g6() {
  $arr = array(0,'a'=>1,2,'b'=>3,4);
  $b = true;
  foreach ($arr as $k => &$v) {
    yield null;
    echo "key=$k val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      array_pop($arr);
    }
  }
}
foreach (g6() as $_) {
}
