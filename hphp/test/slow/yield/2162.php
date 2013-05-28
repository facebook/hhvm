<?php

$env = 3;
$f = function (&$arg0) use ($env) {
  yield $arg0++ + $env;
  yield $arg0++ + $env;
}
;
foreach ($f($env) as $x) {
  var_dump($x);
}
var_dump($env);
