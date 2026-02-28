<?hh

class Super {
  protected function foo() :mixed{
    echo "Super::foo\n";
  }
}

interface I1 {
  require extends Super;

  public function baz():mixed;
}

trait T1 implements I1 {

  public function bar() :mixed{
    return $this->foo();
  }
}

class C {
  use T1;

  public function baz() :mixed{}
}

function main() :mixed{
  $c = new C();
  $c->bar();
}

<<__EntryPoint>>
function main_require_constraint_iface_trait_error() :mixed{
main();
}
