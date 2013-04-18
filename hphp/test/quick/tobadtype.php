<?php ;

class X {
  function not__toString() { throw new Exception('boom'); }
}

function error_handler() {
  throw new Exception("user error handler");
}
//set_error_handler('error_handler');

function foo($x) {
  return $x;
}

function testStr($x) {
  return foo((string)$x);
}

function testInt($x) {
  return foo((int)$x);
}

function testDbl($x) {
  return foo((double)$x);
}

echo "test int\n";

try {
  testInt(new X);
} catch (Exception $e) {
  var_dump($e->getMessage());
}

echo "test dbl\n";

try {
  testDbl(new X);
} catch (Exception $e) {
  var_dump($e->getMessage());
}

echo "test str\n";

try {
  testStr(new X);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
