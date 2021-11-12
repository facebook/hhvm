<?hh

class Bar {
  public readonly int $ro_prop = 3;
  public int $prop = 3;
}
async function returns_5(): readonly Awaitable<int> {
  return readonly 5;
}
<<__EntryPoint>>
async function bar(): Awaitable<void> {
  $x = readonly Vector {};
  // error, $x is not COW
  concurrent {
    $x[] = await readonly returns_5();
    $x[] = await readonly returns_5();
  }
  var_dump($x);
}
