<?php

function f7() {
  $i = 0;
  $bar = array();
  $arr = array(0,1,0,1);
  foreach ($arr as $k => &$v) {
    echo "key=$k value=$v\n";
    if ($k == 0)
      $val = 1;
    else
      $val = $arr[$k-1];
    $arr[$k+1] = $val;
    ++$i;
    if ($i >= 20)
      break;
  }
}

<<__EntryPoint>>
function main_459() {
f7();
}
