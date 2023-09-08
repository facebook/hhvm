<?hh


namespace mojpc2;

interface J {
  public function bar<reify T>(): void;
}

interface I {
  public function foo(): J;
}

function repro(I $i): void {
  $j = $i->foo();
  $j->bar(); // No error
}
