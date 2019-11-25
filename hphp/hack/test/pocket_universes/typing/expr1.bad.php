<?hh // strict
// KO: wrong case value type
class PU {
  enum E {
    case int v;
    :@X (v = 1);
    :@Y (v = 2);
  }

  public static function test(): string {
    return self:@E::v(:@X);
  }
}
