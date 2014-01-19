<?php

$env = 3;
$f = function ($arg0) use (&$env) {
  yield $arg0;
  yield $arg0 + ($env++);
  yield $arg0 + ($env++) + 1;
}
;
foreach ($f(32) as $x) {
 var_dump($x);
 }
var_dump($env);
