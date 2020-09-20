<?hh

interface I1 {
  public function baz();
}

interface I2 {}

class Super {
  protected function foo() {
    echo "Super::foo\n";
  }
}

trait T1 {
  require extends Super;

  require implements I1;

  public function bar() {
    return $this->foo();
  }
}

trait T2 implements I2 {
  use T1;
}
trait T3 {
  use T2;
}

class C extends Super {
  use T3; // fails the requirements of T1
}

function main() {
  $c = new C();
  $c->bar();
}

<<__EntryPoint>>
function main_require_constraint_recurse() {
main();
}
