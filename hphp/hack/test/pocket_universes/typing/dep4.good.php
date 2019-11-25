<?hh // strict

function expect_int(int $_) : void { }
function expect_string(string $_) : void { }
function id<T>(T $x) : T {
  return $x;
}

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
    expect_int(id<int>(static::get(:@I)));
    expect_string(id<string>(static::get(:@S)));
  }
}
