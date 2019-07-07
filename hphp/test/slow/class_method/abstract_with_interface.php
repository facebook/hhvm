<?hh

interface I {
  static function foo();
}

abstract class A implements I {
  <<__NEVER_INLINE>>
  static function bar() {
    return static::foo();
  }
}

abstract class B extends A {
  static function foo() {
    return 42;
  }
}

abstract class C extends A {
  static function foo() {
    return 24;
  }
}

class H extends B {}

<<__NEVER_INLINE>>
function test($o) {
  return $o is I;
}

<<__EntryPoint>>
function main() {
  $h = new H;
  if (test($h)) {
    B::bar();
    C::bar();
  }
  var_dump(B::bar());
}
