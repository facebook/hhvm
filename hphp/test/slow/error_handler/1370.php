<?php

function handler($code, $msg, $file, $line) {
 var_dump($line);
}
set_error_handler('handler');
function f($a) {
  $b = $a[100];
  return $b;
}
f(array(1, 2, 3));
