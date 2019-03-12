<?hh

class X {
  const FOO = Y::BAR;
  const BAZ = 5;
  const BIZ = Y::BAR;
}

if (!isset($g)) {
  include 'class_constants-1.inc';
}

class A {
  const FOO = B::FOO;
  const BAR = "A::BAR";
}
if (!isset($g)) {
  include 'class_constants-2.inc';
}
class C extends A {
}

# Test recursive non-scalar class constant initialization.
class D {
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
  include 'class_constants-3.inc';
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
