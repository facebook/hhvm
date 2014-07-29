<?php

function cow_idx($copy) {
  return idx($copy, '10');
}

function main() {
  $arr = hphp_miarray();
  $arr[10] = 100;
  $res = idx($arr, '10');
  $arr[] = "no warning";

  $arr = hphp_miarray();
  $arr[10] = 100;
  $res = cow_idx($arr);
  $arr[] = "warning";
}

main();
