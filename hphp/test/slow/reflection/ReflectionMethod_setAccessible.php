<?hh
class Foo {
  private function myPrivateMethod() :mixed{
    return 42;
  }
  protected function myProtectedMethod() :mixed{
    return 52;
  }
  public function myPublicMethod() :mixed{
    return 62;
  }
  protected static function myProtectedStaticMethod() :mixed{
    return 72;
  }
}


<<__EntryPoint>>
function main_reflection_method_set_accessible() :mixed{
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
}
