<?php

$in = [['a', 'b'], ['c', 'd']];

foreach ($in as list($herp, $derp)) {
  var_dump($herp);
  var_dump($derp);
}

foreach ($in as [$herp, $derp]) {
  var_dump($herp);
  var_dump($derp);
}

list($a, $b) = ['foo', 'bar'];
var_dump($a);
var_dump($b);

list($a, list($b, $c)) = ['foo', ['bar', 'baz']];
var_dump($a);
var_dump($b);
var_dump($c);

[$a, $b] = ['foo', 'bar'];
var_dump($a);
var_dump($b);

[$a, [$b, $c]] = ['foo', ['bar', 'baz']];
var_dump($a);
var_dump($b);
var_dump($c);

[$a,,$c] = ['foo', 'bar', 'baz'];
var_dump($a);
var_dump($c);
[$a,$_,$c] = ['foo', 'bar', 'baz'];
var_dump($a);
var_dump($c);
