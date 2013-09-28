<?php

function h1(&$arr, $i) {
  foreach ($arr as $k => &$v) {
    echo "i=$i key=$k\n";
    if ($k == 0) {
      if ($i > 0) {
        h1($arr, $i-1);
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
h1($arr, 10);
var_dump($arr);
