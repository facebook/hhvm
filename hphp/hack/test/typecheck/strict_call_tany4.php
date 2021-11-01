<?hh

async function any() /* : TAny */ {
  return new D();
}

class C {
  public function cc(): void {}
}

class D extends C {
  public function dd(): void {}
}

async function expload(): Awaitable<void> {
  $any = any();
  $x = await $any;
  invariant($x is C, '');
  $x->dd();
}
