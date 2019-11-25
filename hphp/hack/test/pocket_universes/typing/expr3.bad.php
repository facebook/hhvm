<?hh // strict
// KO: wrong enum
class PU {
  enum E {
    case int v;
    :@X (v = 1);
    :@Y (v = 2);
  }

  enum F {
    case int v;
    :@X (v = 1);
  }

  public static function test(this:@E $x): int {
    return static:@F::v($x);
  }
}
