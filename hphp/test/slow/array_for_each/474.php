<?php

function g5() {
  $arr = array(0,1,2,3);
  $arr2 = $arr;
  $b = true;
  foreach ($arr as &$v) {
    yield null;
    echo "val=$v\n";
    if ($b && $v == 1) {
      $b = false;
      array_push(&$arr, 4);
    }
  }
}

<<__EntryPoint>>
function main_474() {
foreach (g5() as $_) {
}
}
