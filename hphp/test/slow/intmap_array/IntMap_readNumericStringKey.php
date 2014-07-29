<?php

function main() {
  $a = hphp_miarray();
  $a[123] = 10;
  $foo = $a["123"];
  var_dump($foo);

  $b = hphp_miarray();
  $b[10] = "10";
  $foo = "10";
  $foo = $b[$foo];
  var_dump($foo);
}

main();
