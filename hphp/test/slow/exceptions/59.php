<?php

function foo($a, $b) {
 return $a + $b;
 }
function myErrorHandler($errno, $errstr, $errfile, $errline) {
  var_dump($errstr, $errline);
}
$old_error_handler = set_error_handler('myErrorHandler');
function bar($a, $b) {
  if ($a) {
    $value = $a * foo(1, 2);
  }
  return 1 / $b;
}
set_error_handler('myErrorHandler');
$r = bar(1, 0);
