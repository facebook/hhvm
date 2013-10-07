<?php
function f() {
  $arr2 = array(0 => 'a');
  $arr1 = array(0 => 'a', 1 => 'b');
  unset($arr1[0]);
  reset($arr1);
  reset($arr2);
  var_dump(current($arr1));
  $arr1 += $arr2;
  var_dump(current($arr1));
}
f();

