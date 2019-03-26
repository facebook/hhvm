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

function test(&$a, &$b) {
  $c = 'abc';
  blah('a', &$a, &$b, &$c);
  var_dump($a);
}


<<__EntryPoint>>
function main() {
  $a = 123;
  test(&$a, &$a);
  unset($a);
  set_error_handler('error');
  try {
    $a = 123;
    test(&$a, &$a);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
}
