<?php

function main($arr1, $arr2) {
  $tot = 0;
  foreach($arr1 as $v1) {
    foreach($arr2 as $v2) {
      $tot += $v1 * $v2;
    }
  }
  return $tot;
}

$a = array(1,2,3);
$b = array(10,20,30);

for ($i = 0; $i < 10; $i++) {
  var_dump(main($a, $b));
}
