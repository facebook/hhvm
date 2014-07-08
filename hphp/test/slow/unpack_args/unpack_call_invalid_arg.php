<?php

function f($a, $b, ...$c) {
  echo __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function main() {
  $args = null;
  f('a', 'b', ...$args);
  $args = new stdClass();
  f('a', 'b', ...$args);
  // FIXME(t4599379): This is a Traversable
  $args = new ArrayIterator(['c', 'd', 'e']);
  f('a', 'b', ...$args);
}

main();
