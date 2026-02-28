<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A { }
class B { }
class C<T> { }
class D extends C<B> { }

interface I<+Ti> { }
interface J<+Tj> { }
class E implements I<A>, J<B> { }

function does_is_check<Tc as C<T>,T>(Tc $tc, T $t): T {
  if ($tc is D) {
    return new B();
  }

  return $t;
}

function another<Ti as I<T> as J<T>, T>(bool $b, Ti $ti, T $t): T {
  if ($ti is E) {
    return $b ? new A() : new B();
  }
  return $t;
}
