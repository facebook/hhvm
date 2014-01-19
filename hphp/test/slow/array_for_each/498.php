<?php

function k4() {
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
  var_dump(current($arr2));
  var_dump(current($arr));
}
k4();
