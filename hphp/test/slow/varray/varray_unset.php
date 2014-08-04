<?php

function cow_unset($arr) {
  unset($arr[0]);
}

function main() {
  $a = hphp_varray();
  $a[0] = 1;
  $a[1] = 2;
  unset($a[0]); // warning
  var_dump($a);
  unset($a[1]); // no warning
  var_dump($a);

  $a = hphp_varray();
  $a[0] = 1;
  $a[1] = 2;
  cow_unset($a); // warning
}

main();
