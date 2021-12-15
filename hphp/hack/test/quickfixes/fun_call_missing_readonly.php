<?hh

class Foo {
}

function returns_readonly(): readonly Foo {
  return new Foo();
}

function call_it(): void {
  $ro = returns_readonly();
}
