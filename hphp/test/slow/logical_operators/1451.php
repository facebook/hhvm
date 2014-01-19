<?php

function foo() {
 echo "foo";
 }
$a = (false && foo());
$b = (true  || foo());
$c = (false and foo());
$d = (true  or  foo());
$e = ($e || true);
$f = ($f or true);
$g = ($g && false);
$h = ($h and false);
var_dump($a, $b, $c, $d, $e, $f, $g, $h);
