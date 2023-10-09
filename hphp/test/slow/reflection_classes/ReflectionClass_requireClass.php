<?hh

namespace Test;

abstract class TestBase{}

interface ITestRequiredInterface {}

trait TTestTrait {

  require class TestClass;
  require implements ITestRequiredInterface;
  require extends TestBase;
}

final class TestClass extends TestBase implements ITestRequiredInterface {
  use TTestTrait;
}


<<__EntryPoint>>
function main_reflection_class_required_class() :mixed {
  $reflection = new \ReflectionClass('\Test\TestBase');
  \var_dump($reflection->getRequiredClass());


  $reflection = new \ReflectionClass('\Test\ITestRequiredInterface');
  \var_dump($reflection->getRequiredClass());


  $reflection = new \ReflectionClass('\Test\TTestTrait');
  \var_dump($reflection->getRequiredClass());

  $reflection = new \ReflectionClass('\Test\TestClass');
  \var_dump($reflection->getRequiredClass());
}
