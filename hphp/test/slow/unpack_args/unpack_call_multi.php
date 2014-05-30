<?php

function f($a, $b, $c) {
  echo __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function main() {
  $args = ['a', 'b', 'c'];
  f(...$args, ...$args);
}

main();
