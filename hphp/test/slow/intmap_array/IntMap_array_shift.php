<?php

function main() {
  $a = hphp_miarray();
  $a[10] = 10;
  array_shift($a);
}

main();
