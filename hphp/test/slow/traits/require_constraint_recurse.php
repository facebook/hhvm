<?hh

interface I1 {
  public function baz();
}

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

trait T2 {
  use T1;
}
trait T3 {
  use T2;
}

class C extends Super {
  use T3;
}

function main() {
  $c = new C();
  $c->bar();
}
main();
