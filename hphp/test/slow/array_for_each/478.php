<?php

function g7() {
  $arr = array(0,'a'=>1,2,'b'=>3,4);
  $b = true;
  foreach ($arr as $k => &$v) {
    yield null;
    echo "key=$k val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      unset($arr[1]);

    }
  }
}
foreach (g7() as $_) {
}
