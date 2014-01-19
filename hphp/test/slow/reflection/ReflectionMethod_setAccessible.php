<?php
class Foo {
  private function myPrivateMethod() {
    return 42;
  }
  protected function myProtectedMethod() {
    return 52;
  }
  public function myPublicMethod() {
    return 62;
  }
  protected static function myProtectedStaticMethod() {
    return 72;
  }
}

$method = new ReflectionMethod('Foo', 'myPrivateMethod');
$method->setAccessible(true);
var_dump($method->invoke(new Foo()));
$method = new ReflectionMethod('Foo', 'myProtectedMethod');
$method->setAccessible(true);
var_dump($method->invoke(new Foo()));
$method->setAccessible(false);
try {
  $method->invoke(new Foo());
}
catch (ReflectionException $ex) {
  echo "Exception\n";
}
$method = new ReflectionMethod('Foo', 'myPublicMethod');
$method->setAccessible(false);
var_dump($method->invoke(new Foo()));
$method = new ReflectionMethod('Foo', 'myProtectedStaticMethod');
$method->setAccessible(true);
var_dump($method->invoke(new Foo()));
