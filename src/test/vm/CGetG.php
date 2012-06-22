<?php

$n = 10;
$vals = array();
$a = array();

function main() {
  for ($i = 0; $i < 10; $i++) {
    $vals[] = $GLOBALS['n'];
  }
  var_dump($vals);

  for ($i = 0; $i < 10; $i++) {
    $gname = "a" . (string)$i;
    $GLOBALS[$gname] = $i;
  }

  printf("%016x\n", 1 << $GLOBALS['a0']);
  printf("%016x\n", 1 << $GLOBALS['a1']);
  printf("%016x\n", 1 << $GLOBALS['a2']);
  printf("%016x\n", 1 << $GLOBALS['a3']);

  $GLOBALS[42] = "---42---";
  $a[] = $GLOBALS[42];
  $a[] = $GLOBALS["42"];
  var_dump($a);
}

main();
