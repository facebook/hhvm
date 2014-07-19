<?php

function main() {
  $a = hphp_miarray();
  $a[1] = 2;
  $a[100] = 200;
  $b = array_map($x ==> $x / 2,
                 $a);
  var_dump($b);
  var_dump($a);
}

main();
