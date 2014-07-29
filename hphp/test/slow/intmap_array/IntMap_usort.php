<?php

function main() {
  $a = hphp_miarray();
  $a[1] = 1;
  $a[200] = 0;
  usort($a,
        function($x, $y) {
          if ($x < $y) {
            return -1;
          } else {
            return 1;
          }
        });
  var_dump($a);
}

main();
