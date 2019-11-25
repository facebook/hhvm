<?hh // strict
// KO: wrong inherit (wrong direction)
class PU1 {
  enum E {
    case int v;
  }
}

class PU2 extends PU1 {
  enum E {
  }

  public static function test(PU2:@E $x): int {
    return PU1:@E::v($x);
  }
}
