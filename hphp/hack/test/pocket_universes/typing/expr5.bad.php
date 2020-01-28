<?hh // strict
// KO: wrong case value type
class PU {
  enum E {
    case int value;
    :@X (value = 1);
  }

  public static function test(): string {
    return self:@E::v(:@X);
  }
}
