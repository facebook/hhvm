<?hh

class AAAAA {}
final class BBBBB<+T> extends AAAAA {
  private ?T $x;
}

function wildcard_hint_is_expr(AAAAA $x): bool {
  if ($x is BBBBB<_>) {
    return true;
  }
  return false;
}
