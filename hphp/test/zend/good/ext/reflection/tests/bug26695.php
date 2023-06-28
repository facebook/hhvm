<?hh

class Foo {
}

class Bar {
  function demo(Foo $f) :mixed{
  }
}
<<__EntryPoint>> function main(): void {
$class = new ReflectionClass('Bar');
$methods = $class->getMethods();
$params = $methods[0]->getParameters();

$class = $params[0]->getClass();

var_dump($class->getName());
echo "===DONE===\n";
}
