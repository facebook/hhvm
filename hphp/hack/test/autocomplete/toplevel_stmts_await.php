<?hh

class Foo {
  public function meth(): string {
    return "";
  }
}

async function make(): Awaitable<Foo> {
  return new Foo();
}

$foo = await make();
$foo->mAUTO332
