<?hh

abstract class C1 {}

interface I1<T> {
  require extends C1;

  public function foo(): T;
}

abstract class C2 extends C1 implements I1<string> {}

interface I2 {
  require extends C2;

  public function bar();
}

interface I3 extends I2 {
  public function bar();
}

abstract class C3 extends C2 implements I3 {}

class C4 extends C3 {

  public function foo(): string {
    echo __METHOD__, "\n";
    return __METHOD__;
  }

  public function bar() {
    echo __METHOD__, "\n";
  }
}

function main() {
  $i = new C4();
  $i->foo();
  $i->bar();
}

function reflect_requirements($name) {
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

function reflection() {
  echo '-------', __FUNCTION__, '-------', "\n";
  reflect_requirements(I3::class);
  reflect_requirements(C3::class);
  reflect_requirements(C4::class);
}
main();
reflection();
