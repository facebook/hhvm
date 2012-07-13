<?php
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
