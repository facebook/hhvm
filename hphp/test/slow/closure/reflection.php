<?hh

class Foo {
  static public function getClosure() {
    return function() { return "hi"; };
  }
}

$func = Foo::getClosure();
$reflection = new \ReflectionFunction($func);
$result = $reflection->invokeArgs([]);
var_dump($result);
