<?php

function numeric_cow($copy) {
  $copy['123'] = 10;
  return $copy;
}

function main() {
  $a = hphp_miarray();
  $a["123"] = 10;
  $a["456"] = 20;
  var_dump($a);

  $a = hphp_miarray();
  $foo = "123";
  $a[$foo] = 10;
  $a[$foo] = 20;
  var_dump($a);

  $a = hphp_miarray();
  $b = numeric_cow($a);
  var_dump($b);
  $a[] = "warning";
}

main();
