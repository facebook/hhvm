<?hh

class DEF {
  const type T = int;
}

class ABC extends DEF {
  public function taccess_parent(parent::T $x): void {}
}
