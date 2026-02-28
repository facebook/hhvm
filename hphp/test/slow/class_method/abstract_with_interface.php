<?hh

interface I {
  static function foo():mixed;
}

abstract class A implements I {
  <<__NEVER_INLINE>>
  static function bar() :mixed{
    return static::foo();
  }
}

abstract class B extends A {
  static function foo() :mixed{
    return 42;
  }
}

abstract class C extends A {
  static function foo() :mixed{
    return 24;
  }
}

class H extends B {}

<<__NEVER_INLINE>>
function test($o) :mixed{
  return $o is I;
}

<<__EntryPoint>>
function main() :mixed{
  $h = new H;
  if (test($h)) {
    B::bar();
    C::bar();
  }
  var_dump(B::bar());
}
