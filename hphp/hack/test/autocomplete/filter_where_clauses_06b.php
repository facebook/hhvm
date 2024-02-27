<?hh

class ExpectObj<T> {
  final public function toEqual(T $expected): void {}
  final public function greaterThan(num $expected): void where T as num {}
}

class E<T1,T2> extends ExpectObj<T1> {}

function myTest() {
  new E<int,int>->AUTO332
}
