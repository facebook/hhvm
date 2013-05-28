<?php

function k3() {
  $arr = array(0,1,2,3,4);
  reset($arr);
  $b = true;
  foreach ($arr as $v) {
    if ($b) {
      $b = false;
      $arr2 = $arr;
    }
    echo "val=$v\n";
  }
  var_dump(current($arr));
  var_dump(current($arr2));
}
k3();
