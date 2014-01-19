<?php

function foo($a) {
  $flag = true;
  try {
    f($a);
    $flag = false;
  }
 catch (Exception $e) {
  }
  var_dump($flag);
}
function f($a) {
 if ($a) throw new Exception('What?');
 }
foo(1);
