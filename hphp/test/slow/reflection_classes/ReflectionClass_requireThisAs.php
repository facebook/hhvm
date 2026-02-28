<?hh

namespace Test;

<<file:__EnableUnstableFeatures('require_constraint')>>

abstract class TestBase {}

interface ITestRequiredInterface {}

trait TTestTrait {
  require this as TestClass;
  require implements ITestRequiredInterface;
  require extends TestBase;
}

class TestClass extends TestBase implements ITestRequiredInterface {
  use TTestTrait;

  public function bar(): void {}
}


<<__EntryPoint>>
function main_reflection_class_required_this_as_class(): mixed {
  $reflection = new \ReflectionClass('\Test\TestBase');
  \var_dump($reflection->getRequiredThisAsClass());

  $reflection = new \ReflectionClass('\Test\ITestRequiredInterface');
  \var_dump($reflection->getRequiredThisAsClass());

  $reflection = new \ReflectionClass('\Test\TTestTrait');
  \var_dump($reflection->getRequiredThisAsClass());

  $reflection = new \ReflectionClass('\Test\TestClass');
  \var_dump($reflection->getRequiredThisAsClass());
}
