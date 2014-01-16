<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

function foo() {
  global $foo;
  return $foo;
}
foo();
$x = array_keys($GLOBALS);
sort($x);
foreach ($x as $k) {
  echo "$k=>" . $GLOBALS[$k] . "\n";
}
