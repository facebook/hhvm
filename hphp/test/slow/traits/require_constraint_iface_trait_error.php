<?hh

class Super {
  protected function foo() {
    echo "Super::foo\n";
  }
}

interface I1 {
  require extends Super;

  public function baz();
}

trait T1 implements I1 {

  public function bar() {
    return $this->foo();
  }
}

class C {
  use T1;

  public function baz() {}
}

function main() {
  $c = new C();
  $c->bar();
}
main();
