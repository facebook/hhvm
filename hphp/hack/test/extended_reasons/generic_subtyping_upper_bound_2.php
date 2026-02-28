<?hh


class A {}
class B extends A {}
class C extends B {}
class D extends B {}

function rcvr_both_bounds<T as D as B>(T $x): void {}

function call_both_bounds_with_upper_bound<T as A>(T $x): void {
  rcvr_both_bounds($x);
}
