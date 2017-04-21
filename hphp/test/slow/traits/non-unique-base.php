<?hh // strict

if (getenv("NOFOO")) {
  class B {}
} else {
  class B {
    protected function foo() { echo "hi from foo\n"; }
  }
}

trait T {
  abstract protected function foo();
  public function main() { $this->foo(); }
}

class C extends B {
  use T;
}

$obj = new C;
$obj->main();
