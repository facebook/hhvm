<?php

$y = 'y';
$f = function($x) use ($y) {
  static $z = '';
  $z .= 'z';
  return $x . $y . $z . "\n";
};

function g($func) {
  return $func('x');
}

echo $f('x');
echo g(clone $f);
echo g(clone $f);
