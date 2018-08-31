<?php

function g($func) {
  return $func('x');
}


<<__EntryPoint>>
function main_clone() {
$y = 'y';
$f = function($x) use ($y) {
  static $z = '';
  $z .= 'z';
  return $x . $y . $z . "\n";
};

echo $f('x');
echo g(clone $f);
echo g(clone $f);
}
