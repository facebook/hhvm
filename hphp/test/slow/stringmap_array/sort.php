<?php

function cow_sort($arr) {
  sort($arr);
}

function main() {
  $a = hphp_msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;

  sort($a);
  $a[] = 'no warning';

  $a = hphp_msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;
  cow_sort($a);
  $a[] = "warning";
}

main();
