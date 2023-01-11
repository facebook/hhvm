<?hh

class A {}

class B {}

class C<T> extends B {}

class D<T> extends C<T> {}

class E<T> extends C<int> {}

interface I<T> {
  require extends C<int>;
}

interface J<T> {
  require extends C<T>;
}

/* HH_FIXME[4101] */
function f(C $x) : B {
  return $x; // OK
}

/* HH_FIXME[4101] */
function g(B $x) : C {
  return $x; // ERROR
}

/* HH_FIXME[4101] */
function h(C $x) : C<int> {
  return $x; // ERROR
}

/* HH_FIXME[4101] */
function i(C<int> $x) : C {
  return $x; // OK
}

/* HH_FIXME[4101] */
function j(C $x) : A {
  return $x; // ERROR
}

/* HH_FIXME[4101] */
function k(A $x) : C {
  return $x; // ERROR
}

/* HH_FIXME[4101] */
function l(D $x) : C<int> {
  return $x; // ERROR
}

/* HH_FIXME[4101] */
function m(E $x) : C<int> {
  return $x; // OK
}

/* HH_FIXME[4101] */
function n(E<int> $x) : C {
  return $x; // OK
}

/* HH_FIXME[4101] */
function o(D<int> $x) : C {
  return $x; // OK
}

/* HH_FIXME[4101] */
function p(I $x) : C<int> {
  return $x; // OK
}

/* HH_FIXME[4101] */
function q(J $x) : C<int> {
  return $x; // ERROR
}
