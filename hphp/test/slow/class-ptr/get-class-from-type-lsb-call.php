<?hh

class C {
  public static function f(): void { echo "called f\n"; }
}

class R<reify T> {
  public static function g(): void { echo "called g\n"; }
}

abstract class DAbs {
  abstract const type T;
  abstract const type U;

  public function call_t(): void {
    $c = HH\ReifiedGenerics\get_class_from_type<this::T>();
    $c::f();
  }

  public function call_u(): void {
    $r = HH\ReifiedGenerics\get_class_from_type<this::U>();
    $r::g();
  }

  public static function scall_t(): void {
    $c = HH\ReifiedGenerics\get_class_from_type<this::T>();
    $c::f();
  }

  public static function scall_u(): void {
    $r = HH\ReifiedGenerics\get_class_from_type<this::U>();
    $r::g();
  }
}

class D extends DAbs {
  const type T = C;
  const type U = R<int>;
}

<<__EntryPoint>>
function main(): void {
  $d = new D();
  $d->call_t();
  D::scall_t();

  $d->call_u();
  D::scall_u();
}
