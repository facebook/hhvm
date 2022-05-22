<?hh

interface I {
  public function foo(): void;
}

function bar(I $i): void {
  $i->foo();
}
