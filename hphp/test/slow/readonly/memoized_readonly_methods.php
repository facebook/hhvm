<?hh

class Foo {
  <<__Memoize>>
  public readonly function getArchitecture(int $x)[]: void {
  }

}
async function genReturnFoo(): Awaitable<Foo> {
  return new Foo();
}

<<__EntryPoint>>
async function test(): Awaitable<void> {
  $x = readonly await genReturnFoo();
  $x->getArchitecture(5);
  echo "Done\n";
}
