<?hh

class AAAAA {}
final class BBBBB<+T> extends AAAAA {
  private ?T $x;
}

function wildcard_hint_is_expr(AAAAA $x): void {
  $_ = $x as BBBBB<_>;
}
