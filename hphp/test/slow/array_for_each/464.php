<?php

function f9() {
  $i = 0;
  $arr = array(1,1,1);
  $bar = array();
  $first = true;
  foreach ($arr as $k => &$v) {
    yield null;
    echo "k=$k v=$v\n";
    if (!$first) {
      $prev_k = ($k+2)%3;
      unset($arr[$prev_k]);
      if (count($bar) > 100)
        $bar = array();
      $bar[] = 1;
      $arr[$prev_k] = 1;
    }
    $first = false;
    ++$i;
    if ($i >= 20)
      break;
  }
}
foreach (f9() as $_) {
}
