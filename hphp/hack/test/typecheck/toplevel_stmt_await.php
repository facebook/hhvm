<?hh

class C {
  public function foo(): string {
    return "";
  }
}

async function make(): Awaitable<C> {
  return new C();
}

$c = await make();
$c->foo() + 2;
