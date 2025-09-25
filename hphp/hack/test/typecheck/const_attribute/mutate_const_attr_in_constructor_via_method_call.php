<?hh

class C {
  <<__Const>>
  public int $b = 0;

  public function __construct(<<__Const>> public int $a) {
    $this->b = 0;
    $this->incrB();
  }

  public function incrB(): void {
    $this->b++;
  }

  public function incrA(): void {
    $this->a++;
  }

  public function resetA(): void {
    $this->a = 0;
  }
}

<<__EntryPoint>>
function main(): void {
  $c = new C(4);
  // Bug: not a type error, but we expect that value types
  // cannot be reassigned. HHVM enforces this at runtime.
  $c->incrB();
  $c->incrA();
  $c->b++;
  $c->a++;

  // Correctly a type error
  $c->b = 5;
  $c->a = 10;
  $c->resetA();
}
