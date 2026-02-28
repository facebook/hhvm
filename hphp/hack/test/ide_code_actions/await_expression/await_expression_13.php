<?hh

class Foo() {
  public function meth(): int {
    return 1;
  }
}

async function gen_foo(): Awaitable<Foo> {
  return new Foo();
}

function foo(): void {
  // The refactoring should generate `(await gen_foo())->meth()`
  /*range-start*/gen_foo()/*range-end*/->meth();
}
