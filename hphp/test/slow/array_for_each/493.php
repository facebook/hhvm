<?php

function foo() {
  $arr = array(10,20,30,40,50);
  foreach ($arr as $k => &$v) {
    echo $k . "\n";
    if ($k == 2 && !isset($arr2)) {
      $arr2 = $arr;
      $arr[] = 60;
    }
    $v += 100;
  }
  var_dump($arr);
  var_dump($arr2);
}

<<__EntryPoint>>
function main_493() {
foo();
}
