<?hh // strict

function expect_int(int $_) : void { }
function expect_string(string $_) : void { }

class PU {
  enum E {
    case type T;
    case T v;
  }

  public static function dummy(): this:@E:@A:@T {
    return 1664;
  }
}
