<?php /* binding assignment */
// Copyright 2004-present Facebook. All Rights Reserved.

function error($errno, $errstr) {
  if (strpos($errstr, "Hack Array Compat:") === 0) {
    throw new Exception($errstr);
  }
}

function blah($a, &...$b) {
  $b[1] = 456;
}

function test() {
  $a = 123;
  $b = &$a;
  $c = 'abc';
  blah('a', &$a, &$b, &$c);
  var_dump($a);
}


<<__EntryPoint>>
function main_variadic_by_ref() {
test();
set_error_handler('error');
try {
  test();
} catch (Exception $e) {
  echo "Exception: {$e->getMessage()}\n";
}
}
