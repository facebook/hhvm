<?hh

function un_readonly<T>(readonly T $t): T {
  $g = generator($t);
  $g->next();
  return $g->current();
}

function generator<T>(readonly T $n): Generator<int, T, void> {
  // should error here
  yield $n;
}

class Foo {
  public function __construct(public int $prop) {}
}

<<__EntryPoint>>
function main(): void {
  $ro = readonly new Foo(2);
  $mut = un_readonly($ro);
  $mut->prop = 3; // should not be allowed: mutation of a readonly value
}
