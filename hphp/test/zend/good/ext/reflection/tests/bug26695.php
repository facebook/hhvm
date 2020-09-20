<?hh

class Foo {
}

class Bar {
  function demo(foo $f) {
  }
}
<<__EntryPoint>> function main(): void {
$class = new ReflectionClass('bar');
$methods = $class->getMethods();
$params = $methods[0]->getParameters();

$class = $params[0]->getClass();

var_dump($class->getName());
echo "===DONE===\n";
}
