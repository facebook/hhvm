<?php

function foo($x) {
  $r = array();
  foreach ($x as $v) { $r[] = $v; }
  return $r;
}

function main() {
  $heh = foo(array(1,2,3));
  foreach ($heh as $v) { var_dump($v); }
  echo "--\n";
  $heh = foo(array());
  foreach ($heh as $v) { var_dump($v); }
}

main();
echo "done\n";
