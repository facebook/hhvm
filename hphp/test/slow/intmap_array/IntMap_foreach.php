<?php

function main() {
  $a = hphp_miarray();
  $a[0] = 1;
  $a[1] = 2;
  foreach ($a as $key => $val) {
    var_dump($key);
    var_dump($val);
  }
  foreach ($a as $val) {
    var_dump($val);
  }
}

main();
