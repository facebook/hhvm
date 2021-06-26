<?hh

class Thing {
  <<__DynamicallyCallable>> public static function testOne() { echo "one\n"; }
  <<__DynamicallyCallable>> public static function testTwo() { echo "two\n"; }
  <<__DynamicallyCallable>> public static function testThree() { echo "three\n"; }
  <<__DynamicallyCallable>> public static function testFour() { echo "four\n"; }
}

<<__EntryPoint>> function main(): void {
$class = new ReflectionClass('Thing');
$methods = $class->getMethods();
foreach ($methods as $method) {
  var_dump($method->getName());
  $method->invoke(null);
}
}
