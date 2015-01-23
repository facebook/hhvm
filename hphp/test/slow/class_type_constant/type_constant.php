<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  const type X<T> as ConstVector<T> = ConstVector<T>;
  abstract const type Y<+Tk, -Tv> as KeyTraversable<Tk, Tv>;
}

interface T {
  abstract const type Z;
  abstract const type me as this;
}

final class C implements I {
  const type Y<+Tk, -Tv> = ConstMap<Tk, Tv>;
  const type Z = string;
  const type me = C;

  const type Cint = int;

  // Ensure you can define a constant named type
  const type = 400;

  public static::X $x = ImmVector {};
  public static static::Y $y = Map {};

  public static function test(self::me::Z $x): static::me::Z {
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
