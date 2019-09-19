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
      hh\dynamic_class_meth(C::class, __hhvm_intrinsics\launder_value($v))
    );
  }

  function go() {
    parent::go();
    echo "In ".self::class."\n";
    foreach (['f', 'g', 'h', 'i'] as $f) self::point($f);
  }
}

class B { function go() {} }

class C extends B {
  use T;

  <<__DynamicallyCallable>> function f() { echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> private static function g() { echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> protected static function h() { echo __FUNCTION__."\n"; }
  <<__DynamicallyCallable>> public static function i() { echo __FUNCTION__."\n"; }
}

class H extends C { use T; }

class W extends B { use T; }

<<__EntryPoint>>
function main() {
  (new H)->go();
  (new W)->go();

  echo "In main\n";
  foreach (['f', 'g', 'h', 'i'] as $f) {
    wrap(() ==>
      hh\dynamic_class_meth(C::class, __hhvm_intrinsics\launder_value($f))
    );
  }
}
