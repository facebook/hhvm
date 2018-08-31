<?hh

class Super {
  protected function foo() {
    echo "Super::foo\n";
  }
}

interface I1 {
  require extends Super;

  public function bar();
}

interface I2 extends I1 {}

class C implements I2 {
  public function bar() {
    $this->foo();
  }
}

function main() {
  $c = new C();
  $c->bar();
}

<<__EntryPoint>>
function main_require_constraint_iface_basic_error() {
main();
}
