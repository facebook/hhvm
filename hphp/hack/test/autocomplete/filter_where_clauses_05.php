<?hh

class ExpectObj<T> {
  final public function toEqual(T $expected): void {}
  final public function greaterThan(num $expected): void where T as num {}
}

class ExpectObjString extends ExpectObj<string> {}

function myTest() {
  new ExpectObjString()->AUTO332
}
