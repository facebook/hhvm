<?hh // partial

class Three {
  public function __toString(): string {
    return "3";
  }
}

function test(): int {
  $x = 0;
  /* UNSAFE_EXPR */ eval('$x = '.new Three().';');
  return $x;
}
