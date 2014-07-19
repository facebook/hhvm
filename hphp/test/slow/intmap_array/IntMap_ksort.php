<?php

function main() {
  $a = hphp_miarray();
  $a[1] = 0;
  $a[0] = 1;
  var_dump($a);
  ksort($a);
  var_dump($a);
}

main();
