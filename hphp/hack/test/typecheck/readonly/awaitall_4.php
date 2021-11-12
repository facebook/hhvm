<?hh
class Bar {
  public readonly int $ro_prop = 3;
  public int $prop = 3;
}
function takes_mutable(Bar $x): void {}
async function returns_readonly(): readonly Awaitable<Bar> {
  return readonly new Bar();
}

<<__EntryPoint>>
async function bar(): Awaitable<void> {
  $x = readonly Vector {};
  takes_mutable(await readonly returns_readonly());
}
