<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Foo {
  public function __construct(int $prop = 4) {}
}

<<__EntryPoint>>
  function main() : void {

  }

async function foo() : Awaitable<Foo> {
  return new Foo(0);
}

async function bar(): Awaitable<void> {
  $z = await readonly foo();
  $z->prop = 4; // error $z is readonly
}
