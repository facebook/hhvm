<?php

function foo() {
  $arr = array(10,20,30,40,50);
  foreach ($arr as $k => &$v) {
    yield null;
    echo $k . "\n";
    if ($k == 2 && !isset($arr2)) {
      $arr2 = $arr;
    }
    $v += 100;
  }
  var_dump($arr);
  var_dump($arr2);
}
foreach (foo() as $_) {
}
