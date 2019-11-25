<?hh // strict
class PU {
  enum E {
    case type T;
    case T v;
    :@I (type T = int, v = 42);
    :@S (type T = string, v = "foo");
  }

  public static function get<TF as this:@E>(TF $x): this:@E:@TF:@T {
    return static:@E::v($x);
  }

  public static function main(): void {
    assert(static::get(:@I) === 42);
    assert(static::get(:@S) === "foo");
  }
}
