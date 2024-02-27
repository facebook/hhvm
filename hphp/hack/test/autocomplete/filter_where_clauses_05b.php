<?hh

class ExpectObj<T> {
  final public function toEqual(T $expected): void {}
  final public function greaterThan(num $expected): void where T as num {}
}

class ExpectObjInt extends ExpectObj<int> {}

function myTest() {
  new ExpectObjInt()->AUTO332
}
