<?php

function h1(&$arr, $i) {
  foreach ($arr as $k => &$v) {
    yield null;
    echo "i=$i key=$k\n";
    if ($k == 0) {
      if ($i > 0) {
        foreach (h1($arr, $i-1) as $_) {
}
      }
 else if ($i == 0) {
        echo "Unsetting key 1\n";
        unset($arr[1]);
      }
    }
  }
  end($arr);
}
$arr = array('a','b','c');
foreach (h1($arr, 10) as $_) {
}
var_dump($arr);
