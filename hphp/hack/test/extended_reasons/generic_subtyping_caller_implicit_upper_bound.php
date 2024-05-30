<?hh


class A {}
class B extends A {}

function rcvr_concrete(B $x): void {}

function call_concrete_with_implicit_upper_bound<T>(T $x): void {
  rcvr_concrete($x);
}
