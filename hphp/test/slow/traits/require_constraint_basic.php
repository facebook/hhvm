<?hh

interface I1 {
  public function baz():mixed;
}

class Super {
  protected function foo() :mixed{
    echo get_class($this), " ",__METHOD__, "\n";
  }
}

trait T1 {
  require extends Super;

  require implements I1;

  public function bar() :mixed{
    return $this->foo();
  }
}

class C1 extends Super implements I1 {
  use T1;

  public function baz() :mixed{}
}

class Param<T> {
  public function __construct(private ?T $data = null)[] {}
  protected function foo() :mixed{
    echo get_class($this), " ",__METHOD__, "\n";
  }
}

trait T2<T> {
  require extends Param<T>;

  public function bar() :mixed{
    return $this->foo();
  }
}

class C2 extends Param<string> {
  use T2<string>;
}

function main() :mixed{
  $i = new C1();
  $i->bar();

  $i = new C2();
  $i->bar();
}

function reflect_requirements($name) :mixed{
  $rc = new ReflectionClass($name);
  if ($rc->isInterface()) {
    echo 'interface';
  } else if ($rc->isTrait()) {
    echo 'trait';
  } else if ($rc->isAbstract()) {
    echo 'abstract class';
  } else {
    echo 'class';
  }
  echo ' ', $rc->getName(), ' requires:', "\n";
  var_dump($rc->getRequirementNames());
}

function reflection() :mixed{
  echo '-------', __FUNCTION__, '-------', "\n";
  reflect_requirements(T1::class);
  reflect_requirements(T2::class);
}

<<__EntryPoint>>
function main_require_constraint_basic() :mixed{
main();
reflection();
}
