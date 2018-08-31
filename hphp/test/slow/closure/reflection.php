<?hh

class Foo {
  static public function getClosure() {
    return function() { return "hi"; };
  }
}


<<__EntryPoint>>
function main_reflection() {
$func = Foo::getClosure();
$reflection = new \ReflectionFunction($func);
$result = $reflection->invokeArgs([]);
var_dump($result);
}
