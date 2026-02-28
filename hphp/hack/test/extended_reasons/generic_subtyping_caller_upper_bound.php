<?hh


class A {}
class B extends A {}

function rcvr_concrete(B $x): void {}

function call_conrete_with_upper_bound<T as A>(T $x): void {
  rcvr_concrete($x);
}
