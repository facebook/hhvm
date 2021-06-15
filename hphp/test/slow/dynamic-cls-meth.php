<?hh

function wrap($f) {
  try {
    $f()();
  } catch (InvalidArgumentException $e) {
    echo "EX: ".$e->getMessage()."\n";
  }
}

trait T {
  private function point($v) {
    wrap(() ==>
      HH\dynamic_class_meth(C::class, __hhvm_intrinsics\launder_value($v))
    );
  }

  function go() {
    parent::go();
    echo "In ".self::class."\n";
    foreach (varray['f', 'g', 'h', 'i'] as $f) self::point($f);
  }
}

class R { function go() {} }
class B extends R { use T; }

class C extends B {
  use T;

  <<__DynamicallyCallable>> function f() { echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> private static function g() { echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> protected static function h() { echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> public static function i() { echo __FUNCTION__."\n"; }
}

class H extends C { use T; }

class W extends B { use T; }

abstract class A extends C {
  abstract static function j();
}

<<__EntryPoint>>
function main() {
  (new H)->go();
  (new W)->go();

  echo "In main\n";
  foreach (varray['f', 'g', 'h', 'i'] as $f) {
    wrap(() ==>
      HH\dynamic_class_meth(C::class, __hhvm_intrinsics\launder_value($f))
    );
  }
  foreach (varray['f', 'g', 'h', 'i', 'j'] as $f) {
    wrap(() ==>
      HH\dynamic_class_meth(A::class, __hhvm_intrinsics\launder_value($f))
    );
  }
}
