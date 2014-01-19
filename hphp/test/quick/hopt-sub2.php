<?php
function foo($a, $b) {
  // reg - reg
  return $a - $b;
}

function main() {
  echo foo(5, 7);
  echo "\n";
}

main();
