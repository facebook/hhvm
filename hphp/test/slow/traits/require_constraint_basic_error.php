<?hh


interface I1 {
  public function baz():mixed;
}

class Super {
  protected function foo() :mixed{
    echo "Super::foo\n";
  }
}

trait T1 {
  require extends Super;

  require implements I1;

  public function bar() :mixed{
    return $this->foo();
  }
}

class C implements I1 {
  use T1;

  public function baz() :mixed{}
}

function main() :mixed{
  $c = new C();
  $c->bar();
}

<<__EntryPoint>>
function main_require_constraint_basic_error() :mixed{
main();
}
