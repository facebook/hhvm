<?hh // strict

function expect_int(int $_) : void { }
function expect_string(string $_) : void { }

class PU {
  enum E {
    case type T;
    case T v;
    :@I (type T = int, v = 42);
    :@S (type T = string, v = "foo");
  }

  public static function good(): this:@E:@I:@T {
    return 1664;
  }

  public static function get<TF as this:@E>(TF $x): this:@E:@TF:@T {
    return static:@E::v($x);
  }

  public static function main(): void {
    expect_int(static::get(:@I));
    expect_string(static::get(:@S));
  }
}
