<?hh

enum E: int {
  A = 1;
}

final class C {
  public static function foo(): ?E {
    return E::A;
  }
}

function main(): void {
  $e = C::foo();
  switch ($e) {
    case E::A:
      break;
    case null:
      break;
  }
}
