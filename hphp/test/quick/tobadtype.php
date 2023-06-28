<?hh

class X {
  function not__toString() :mixed{ throw new Exception('boom'); }
}

function error_handler() :mixed{
  throw new Exception("user error handler");
}
//set_error_handler(error_handler<>);

function foo($x) :mixed{
  return $x;
}

function testStr($x) :mixed{
  return foo((string)$x);
}

function testInt($x) :mixed{
  return foo((int)$x);
}

function testDbl($x) :mixed{
  return foo((float)$x);
}
<<__EntryPoint>> function main(): void {
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
}
