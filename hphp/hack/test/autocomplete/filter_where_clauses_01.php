<?hh

class ExpectObj<T> {
  final public function toEqual(T $expected): void {}
  final public function greaterThan(num $expected): void where T as num {}
}

function expect<T>(T $obj): ExpectObj<T> {
  return new ExpectObj<T>();
}

function myTest() {
  expect("hello")->AUTO332
}
