<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case type T;
    case T val;
    :@I(type T = int, val = 42);
    :@F(type T = float, val = 15.0);
  }

  public static function f<TP as this:@E>(TP $x, TP $y) : num {
    $xv = static:@E::val($x);
    $yv = static:@E::val($y);
    return $xv + $yv;
  }

  public static function g(this:@E $x, this:@E $y) : num {
    $xv = static:@E::val($x);
    $yv = static:@E::val($y);
    return $xv + $yv;
  }
}
