<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Bar {
  public readonly int $ro_prop = 3;
  public int $prop = 3;
}
async function returns_5(): readonly Awaitable<int> {
  return readonly 5;
}
<<__EntryPoint>>
async function bar(): Awaitable<void> {
  $x = new Bar();
  // ok
  concurrent {
    $x->ro_prop = await readonly returns_5();
    $x->ro_prop = await readonly returns_5();
  }
  // this desugars into the concurrent block above with $x->prop
  list($x->prop, $x->prop) = vec[await readonly returns_5(), await readonly returns_5()];
}
