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

  public function get_t(): class<mixed> {
    return HH\ReifiedGenerics\get_class_from_type<this::T>();
  }

  public function get_u(): class<mixed> {
    return HH\ReifiedGenerics\get_class_from_type<this::U>();
  }

  public static function sget_t(): class<mixed> {
    return HH\ReifiedGenerics\get_class_from_type<this::T>();
  }

  public static function sget_u(): class<mixed> {
    return HH\ReifiedGenerics\get_class_from_type<this::U>();
  }
}

class D extends DAbs {
  const type T = C;
  const type U = R<int>;
}

<<__EntryPoint>>
function main(): void {
  $d = new D();
  $c1 = $d->get_t();
  $c1::f();

  $c2 = D::sget_t();
  $c2::f();

  $r1 = $d->get_u();
  $r1::g();

  $r2 = D::sget_u();
  $r2::g();
}
