<?php

function foo() {
  $bar = $GLOBALS['asd'];
}
foo();
$x = array_keys($GLOBALS);
sort($x);
foreach ($x as $k) { echo "$k->".$GLOBALS[$k]."\n"; }
