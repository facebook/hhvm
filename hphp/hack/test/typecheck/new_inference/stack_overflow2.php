<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  private ?T $x;

  public function get(): T {
      if ($this->x is nonnull) { return $this->x; }
      else { throw new Exception(); }
  }

  public function set(T $x): void {}
}

class A {}

function f(nonnull $x, A $y): void {
  $a = new Inv();
  $b = new Inv();
  $a->set($x);
  $b->set($x);
  $a->set($b->get());
  $b->set($a->get());

  $a->set($y);
}
