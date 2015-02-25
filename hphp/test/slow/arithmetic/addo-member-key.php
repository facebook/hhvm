<?php


function main($arr, $a, $b) {
  return $arr[$a + $b];
}
var_dump(main(array(1, 2, 3, 4, 5, 6), 2, 1));
