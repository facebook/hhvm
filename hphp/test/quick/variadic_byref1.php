<?php

function variadic_by_ref(&...$args) {
  for ($i = 0; $i < count($args); ++$i) { $args[$i]++; }
}

function main() {
  $a = 10;
  $b = 20;
  variadic_by_ref(&$a, &$b);
  var_dump($a, $b);
}
main();
