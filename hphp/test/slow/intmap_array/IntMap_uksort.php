<?php

function main() {
  $a = hphp_miarray();
  $a[200] = 3;
  $a[5] = 49;
  var_dump($a);
  uksort($a, function($x, $y) {
      if ($x < $y) {
        return -1;
      } else {
        return 1;
      }
    });
  var_dump($a);
}

main();
