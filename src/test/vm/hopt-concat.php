<?php
function foo($a, $b) {
  return $a . $b;
}

echo foo('abc', 'def');
echo "\n";
echo foo('abc', 1);
echo "\n";
echo foo(2, 'def');
echo "\n";
echo foo(1, 2);
echo "\n";
echo foo(foo('abc', 123), 'def');
echo "\n";
