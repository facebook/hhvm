<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

function expect_int(int $_) : void { }
function expect_string(string $_) : void { }

class PU {
  enum E {
    case type T;
    case T v;
    :@I (type T = int, v = 42);
  }

  public static function dummy(): A:@T {
    return 1664;
  }

  public static function error_on_atom(): :@I:@T {
    return 64;
  }
}
