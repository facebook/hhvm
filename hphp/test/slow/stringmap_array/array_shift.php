<?php

function cow_shift($arr) {
  return array_shift($arr);
}

function main() {
  $a = hphp_msarray();
  $a['foo'] = 10;
  $b = array_shift($a);
  $a[] = "no warning";

  $a = hphp_msarray();
  $a['foo'] = 10;
  $b = cow_shift($a);
  $a[] = "warning";
}

main();
