<?php

function k2() {
  $arr = array(0,1,2,3,4);
  reset($arr);
  $arr2 = $arr;
  foreach ($arr as $v) {
    echo "val=$v\n";
  }
  var_dump(current($arr));
  var_dump(current($arr2));
}
k2();
