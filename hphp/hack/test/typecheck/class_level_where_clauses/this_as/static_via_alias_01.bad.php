<?hh

newtype B = C;

class C where this as B {
  public function foo(): int { return $this::bar(); }
}
