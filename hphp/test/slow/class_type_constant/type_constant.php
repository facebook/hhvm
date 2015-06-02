<?hh // strict


interface I {
  const type X<T> as ConstVector<T> = ConstVector<T>;
  abstract const type Y<+Tk, -Tv> as KeyTraversable<Tk, Tv>;
}

interface T {
  abstract const type Z;
  abstract const type me as this;
}

abstract class P {
  const type Z as arraykey = arraykey;
}

final class C extends P implements I {
  const type Y<+Tk, -Tv> = ConstMap<Tk, Tv>;
  const type Z = string;
  const type me = C;

  const type Cint = int;

  // Ensure you can define a constant named type
  const type = 400;

  public this::X $x = ImmVector {};
  public static this::Y $y = Map {};

  public static function test(self::me::Y $x): Map<int, this::me::Z> {
    return static::$y;
  }
}

final class D {
  const C::Cint type = 200;
}

define('type', 123);
var_dump(type + 123);
var_dump(type - 123);
var_dump(C::type + D::type);
var_dump(C::type - D::type);
