<?php

function cow_usort($arr) {
  usort($arr, ($x, $y) ==> ($x === $y) ? 0 : ($x < $y ? -1 : 1));
}

function main() {
  $a = hphp_msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;

  usort($a, ($x, $y) ==> ($x === $y) ? 0 : ($x < $y ? -1 : 1));
  $a[] = 'no warning';

  $a = hphp_msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;
  cow_usort($a);
  $a[] = "warning";
}

main();
