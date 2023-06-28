<?hh

class Foo {
  static public function getClosure() :mixed{
    return function() { return "hi"; };
  }
}


<<__EntryPoint>>
function main_reflection() :mixed{
$func = Foo::getClosure();
$reflection = new \ReflectionFunction($func);
$result = $reflection->invokeArgs(varray[]);
var_dump($result);
}
