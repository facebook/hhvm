<?hh

trait A {
  private function f(): void {
    echo "from A\n";
  }

  public function main(): void {
    $this->f();
  }
}

class B {
  use A;

  public function f(int $x): int {
    echo "from B\n";
    return $x + 1;
  }
}

<<__EntryPoint>>
function main(): void {
  $b = new B();
  $b->main();
}
