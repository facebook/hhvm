<?php
function foo($a, $b) {
  // reg - const
  $b = 7;
  return $a - $b;
}

function main() {
  echo foo(5, 7);
  echo "\n";
}

main();
