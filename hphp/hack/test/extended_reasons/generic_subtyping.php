<?hh


class A {}
class B extends A {}
class C extends B {}
class D extends B {}
class E extends D  {}

function rcvr_concrete(B $x): void {}

function call_concrete_with_implicit_upper_bound<T>(T $x): void {
  rcvr_concrete($x);
}

function call_conrete_with_upper_bound<T as A>(T $x): void {
  rcvr_concrete($x);
}

function rcvr_upper_bound<T as B>(T $x): void {}

function call_upper_bound_with_conrete(A $x): void {
  rcvr_upper_bound($x);
}

function call_upper_bound_with_upper_bound<T as A>(T $x): void {
  rcvr_upper_bound($x);
}

function rcvr_both_bounds<T as D as B>(T $x): void {}

function call_both_bounds_with_upper_bound<T as A>(T $x): void {
  rcvr_both_bounds($x);
}
