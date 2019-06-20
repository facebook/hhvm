<?hh

class A {}

class B<T> {
  final public function toNotBeNull<Tu>(): Tu where T = ?Tu {
    throw new Exception();
  }
}

function expect<T>(T $x): B<T> {
  return new B();
}

function test(?A $a): void {
  expect($a)->toNotBeNull();
}
