<?php

function k1() {
  $arr = array(0,1,2,3,4);
  reset($arr);
  foreach ($arr as $v) {
    echo "val=$v\n";
  }
  var_dump(current($arr));
}
k1();
