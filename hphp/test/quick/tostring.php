<?php ;

class X {
  function not__toString() { throw new Exception('boom'); }
}

function foo($x) {
  return $x;
}

function test($x) {
  return foo((string)$x);
}

try {
  test(new X);
} catch (Exception $e) {
  var_dump($e->getMessage());
  }
