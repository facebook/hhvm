<?php

function g11() {
  $arr = array(0,1,2,3);
  reset($arr);
  var_dump(current($arr));
  foreach ($arr as &$v) {
    var_dump(current($arr));
  }
  var_dump(current($arr));
}
g11();
