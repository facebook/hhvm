<?hh

class Super {
  protected function foo() :mixed{
    echo "Super::foo\n";
  }
}

interface I1 {
  require extends Super;

  public function bar():mixed;
}

interface I2 extends I1 {}

class C implements I2 {
  public function bar() :mixed{
    $this->foo();
  }
}

function main() :mixed{
  $c = new C();
  $c->bar();
}

<<__EntryPoint>>
function main_require_constraint_iface_basic_error() :mixed{
main();
}
