<?hh

class Foo {
  public function __construct()[] {}
  public function method() :mixed{}
  public function __toString() :mixed{ throw new Exception('No string casts');}
}


<<__EntryPoint>>
function main_reflection_method_construct() :mixed{
$instance = new Foo();

try {
  $b = new ReflectionMethod(null, 'noSuchMethod');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}

try {
  $b = new ReflectionMethod('Foo', 'noSuchMethod');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}

try {
  $b = new ReflectionMethod($instance, 'noSuchMethod');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}

$b = new ReflectionMethod($instance, 'method');
var_dump($b is ReflectionMethod);
$b = new ReflectionMethod('Foo', 'method');
var_dump($b is ReflectionMethod);

// Look for method 'method' in class 'Foo'
var_dump((new ReflectionMethod('Foo', 'method'))->getName());
var_dump((new ReflectionMethod(new Foo(), 'method'))->getName());
var_dump((new ReflectionMethod('Foo::method'))->getName());
// Look for method '' in class 'Foo'
try {
  var_dump((new ReflectionMethod('Foo', null))->getName());
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
}
