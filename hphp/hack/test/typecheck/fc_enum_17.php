<?hh

enum Foo: int {
  X = 1;
}

function f(): void {
  $v = Foo::X;
  $v::getValues();
}
