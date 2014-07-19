<?php

function main() {
  $a = hphp_miarray();
  $a[0] = "efgh";
  $a[1] = "abcd";
  var_dump($a);
  asort($a);
  var_dump($a);
}

main();
