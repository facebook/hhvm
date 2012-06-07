<?php

function &id(&$x) { return $x; }
function id2(&$x) { return $x; }
function main() {
  $y = 1;
  $z =& id(id(id($y)));
  ++$z;
  echo $y;
  echo "\n";
  $y = 12;
  $w =& id2($y);
  $w = 7;
  echo $y;
  echo "\n";
}
main();

