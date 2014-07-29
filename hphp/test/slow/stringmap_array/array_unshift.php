<?php

function cow_unshift($arr) {
  return array_unshift($arr, 1, 2, 3);
}

function main() {
  $a = hphp_msarray();
  array_unshift($a, 1, 2, 3);
  $a[] = "no warning";

  $a = hphp_msarray();
  cow_unshift($a);
  $a[] = "warning";
}

main();
