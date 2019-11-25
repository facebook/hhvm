<?hh // strict
// OK: inherit case value
class PU1 {
  enum E {
    case int v;
  }
}

class PU2 extends PU1 {
  enum E {
  }

  public static function test(PU2:@E $x): int {
    return PU2:@E::v($x);
  }
}
