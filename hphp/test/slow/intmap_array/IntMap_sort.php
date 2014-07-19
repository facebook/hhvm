<?php

function main() {
  $a = hphp_miarray();
  $a[0] = 1;
  $a[1] = 0;
  sort($a);
  var_dump($a);
}

main();
