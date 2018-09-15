<?hh // strict

class Bar {
  public async function maybeFoo(): Awaitable<?Foo> {
    return null;
  }
}

class Foo {}

function maybe_bar(): ?Bar {
  return new Bar();
}

async function test(): Awaitable<void> {
  $bar = maybe_bar();
  $foo = await $bar?->maybeFoo();
  hh_show($foo);
}
