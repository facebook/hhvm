<?hh // strict
// KO: refers to E instead of this:@E
class PU {
  enum E {
    case int v;
    :@X (v = 1);
    :@Y (v = 2);
  }

  public static function test<TF as E>(TF $x): int {
    return static:@E::v($x);
  }
}
