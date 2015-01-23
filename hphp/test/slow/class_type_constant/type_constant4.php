<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
interface I {
  abstract const type T;
  abstract const int type;
}
class C implements I {
  const type T = string;
  const type = 0;

  public static function not_enforced(self::T $x): self::T {
    return 0;
  }
}

function not_enforced(C::T $x): C::T {
  return 0;
}

C::not_enforced(null);
not_enforced(null);
