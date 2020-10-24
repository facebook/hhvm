<?hh

class C extends A {
}

// Test recursive non-scalar class constant initialization.
class D {
}
class E extends D {
}
class F extends E {
}

// Test inheritance of interface constants
interface I {
  const WEE = 123;
}
interface J extends I {
  const WOO = self::WEE;
}
class K implements J {
}
class L implements J {
}
