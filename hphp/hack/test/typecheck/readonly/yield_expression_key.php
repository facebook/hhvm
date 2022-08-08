<?hh

function un_readonly<T>(readonly T $t): T {
  foreach(generator($t) as $k => $_) {
    return $k;
  }
  throw new Error("");
}

function generator<T>(readonly T $t): Generator<T, int, void> {
  // should error here
  yield ($t) => 1;
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
