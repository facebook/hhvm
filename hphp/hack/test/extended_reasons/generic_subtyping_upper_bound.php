<?hh


class A {}
class B extends A {}

function rcvr_upper_bound<T as B>(T $x): void {}

function call_upper_bound_with_upper_bound<T as A>(T $x): void {
  rcvr_upper_bound($x);
}
