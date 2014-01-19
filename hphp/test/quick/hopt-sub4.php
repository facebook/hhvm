<?php
function foo($a, $b) {
  // const - reg
  $a = 5;
  return $a - $b;
}

function main() {
  echo foo(5, 7);
  echo "\n";
}

main();
