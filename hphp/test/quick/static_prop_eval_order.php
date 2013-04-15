<?php
function f() { echo '1 '; }
function g() { echo '3 '; }
function __autoload($cls) {
  echo '2 ';
  if (strtolower($cls) === 'c') {
    class C { public static $x; }
  }
}
$cls = 'C';
$cls::$x[0][f()] = g();
echo "\n";
