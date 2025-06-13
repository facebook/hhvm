<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  protected internal function foo(): void {}
  public function bar(): void {}
  internal function baz(): void {}

  protected internal int $qux;
  public int $quux;
  internal int $corge;
}

<<__EntryPoint>>
function main() {
  echo "is A::foo internal: ";
  var_dump((new ReflectionMethod('A::foo')->isInternalToModule()));
  echo "is A::bar internal: ";
  var_dump((new ReflectionMethod('A::bar')->isInternalToModule()));
  echo "is A::baz internal: ";
  var_dump((new ReflectionMethod('A::baz')->isInternalToModule()));

  echo "is A::qux internal: ";
  var_dump((new ReflectionProperty('A', 'qux')->isInternalToModule()));
  echo "is A::quux internal: ";
  var_dump((new ReflectionProperty('A', 'quux')->isInternalToModule()));
  echo "is A::corge internal: ";
  var_dump((new ReflectionProperty('A', 'corge')->isInternalToModule()));

  echo "is A::foo protected: ";
  var_dump((new ReflectionMethod('A::foo')->isProtected()));
  echo "is A::foo public: ";
  var_dump((new ReflectionMethod('A::foo')->isPublic()));
  echo "is A::baz public: ";
  var_dump((new ReflectionMethod('A::baz')->isPublic()));
  echo "is A::baz protected: ";
  var_dump((new ReflectionMethod('A::baz')->isProtected()));

  echo "is A::qux protected: ";
  var_dump((new ReflectionProperty('A', 'qux')->isProtected()));
  echo "is A::qux public: ";
  var_dump((new ReflectionProperty('A', 'qux')->isPublic()));
  echo "is A::corge public: ";
  var_dump((new ReflectionProperty('A', 'corge')->isPublic()));
  echo "is A::corge protected: ";
  var_dump((new ReflectionProperty('A', 'corge')->isProtected()));
}
