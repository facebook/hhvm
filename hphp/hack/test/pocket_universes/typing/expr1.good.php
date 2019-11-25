<?hh // strict
// OK: apply argument to case value
class PU {
  const type T = string;

  enum E {
    case int v;
    :@X (v = 1);
    :@Y (v = 2);
  }

  public static function test(this:@E $x): int {
    return static:@E::v($x);
  }
}
