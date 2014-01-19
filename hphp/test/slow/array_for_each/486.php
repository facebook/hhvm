<?php

function g11() {
  $arr = array(0,1,2,3);
  reset($arr);
  var_dump(current($arr));
  foreach ($arr as &$v) {
    yield null;
    var_dump(current($arr));
  }
  var_dump(current($arr));
}
foreach (g11() as $_) {
}
