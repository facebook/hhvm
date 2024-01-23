<?hh

class Foo {
  public $x;
  public function __construct($x) { $this->x = $x; }
}


<<__EntryPoint>>
function main_property_constructor(): mixed {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  $a = new Foo(42);

  $gotException = false;
  try {
    $b = new ReflectionProperty(null, null);
  } catch (Exception $ex) {
    $gotException = true;
  }
  var_dump($gotException);

  $gotException = false;
  try {
    $b = new ReflectionProperty(null, 'x');
  } catch (ReflectionException $ex) {
    $gotException = true;
  }
  var_dump($gotException);

  $gotException = false;
  try {
    $b = new ReflectionProperty($a, null);
  } catch (Exception $ex) {
    $gotException = true;
  }
  var_dump($gotException);

  $b = new ReflectionProperty($a, 'x');
  var_dump($b is ReflectionProperty);
}
