<?hh

function &f(): int {
  static $x = 42;
  return $x;
}

class C {
  private int $x = 42;
  public function &f(): int {
    return $this->x;
  }
}
