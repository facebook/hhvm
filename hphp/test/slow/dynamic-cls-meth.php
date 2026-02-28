<?hh

function wrap($f) :mixed{
  try {
    $f()();
  } catch (InvalidArgumentException $e) {
    echo "EX: ".$e->getMessage()."\n";
  }
}

trait T {
  private function point($v) :mixed{
    wrap(() ==>
      HH\dynamic_class_meth(C::class, __hhvm_intrinsics\launder_value($v))
    );
  }

  function go() :mixed{
    parent::go();
    echo "In ".self::class."\n";
    foreach (vec['f', 'g', 'h', 'i'] as $f) self::point($f);
  }
}

class R { function go() :mixed{} }
class B extends R { use T; }

class C extends B {
  use T;

  <<__DynamicallyCallable>> function f() :mixed{ echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> private static function g() :mixed{ echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> protected static function h() :mixed{ echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> public static function i() :mixed{ echo __FUNCTION__."\n"; }
}

class H extends C { use T; }

class W extends B { use T; }

abstract class A extends C {
  abstract static function j():mixed;
}

<<__EntryPoint>>
function main() :mixed{
  (new H)->go();
  (new W)->go();

  echo "In main\n";
  foreach (vec['f', 'g', 'h', 'i'] as $f) {
    wrap(() ==>
      HH\dynamic_class_meth(C::class, __hhvm_intrinsics\launder_value($f))
    );
  }
  foreach (vec['f', 'g', 'h', 'i', 'j'] as $f) {
    wrap(() ==>
      HH\dynamic_class_meth(A::class, __hhvm_intrinsics\launder_value($f))
    );
  }
}
