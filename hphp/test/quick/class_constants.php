<?php

class X {
  const FOO = Y::BAR;
  const BAZ = 5;
  const BIZ = Y::BAR;
}

if (!isset($g)) {
  class Y {
    const BAR = X::BAZ;
  }
  class Z extends X {
    const FOO = 6;
    const WIZ = W::WIZ;
  }
}

class A {
  const FOO = B::FOO;
  const BAR = "A::BAR";
  const WIZ = WIZ;
}
if (!isset($g)) {
  class B {
    const FOO = "B::FOO";
    const BAR = A::BAR;
  }
}
class C extends A {
}

# Test recursive non-scalar class constant initialization.
class D {
  const CD = DCD;
}
class E extends D {
}
class F extends E {
}

# Test inheritance of interface constants
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

function __autoload($cls) {
  class AutoloadedClass {
    const INDEED = 711;
  }
}

function main() {
  print "Test begin\n";

  var_dump(X::BAZ);
  var_dump(X::FOO);
  var_dump(Z::FOO);
  var_dump(Z::BIZ);

  var_dump(A::BAR);
  var_dump(A::FOO);
  var_dump(A::BAR);
  var_dump(B::FOO);
  var_dump(B::BAR);
  var_dump(C::FOO);
  var_dump(C::BAR);

  # Zend apparently stores A::WIZ and C::WIZ separately, so the following should
  # cause them to end up with different values.
  var_dump(C::WIZ);
  define('WIZ', "--- WIZ ---");
  var_dump(A::WIZ);
  var_dump(C::WIZ);

  define('DCD', "DCD");
  var_dump(F::CD);

  var_dump(K::WEE);
  var_dump(K::WOO);
  var_dump(L::WEE);

  var_dump(constant('K::WEE'));
  var_dump(constant('K::WOO'));
  var_dump(constant('L::WEE'));

  # Test forcing autoload.
  var_dump(AutoloadedClass::INDEED);

  print "Test end\n";
  var_dump(D::FakeConstant);

}

main();
