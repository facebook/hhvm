<?hh // strict

if (getenv("NOFOO")) {
  class B {}
} else {
  class B {
    protected function foo(): void { echo "hi from foo\n"; }
  }
}

trait T {
  abstract protected function foo(): void;
  public function main(): void { $this->foo(); }
}

class C extends B {
  use T;
}

$obj = new C();
$obj->main();
