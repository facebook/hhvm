//// partial.php
<?hh

async function any() {
  return new D();
}

//// strict.php
<?hh // strict

class C {
  public function cc(): void {}
}

class D extends C {
  public function dd(): void {}
}

async function expload(): Awaitable<void> {
  $any = any();
  $x = await $any;
  invariant($x instanceof C, '');
  $x->dd();
}
