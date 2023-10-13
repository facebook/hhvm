<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Ref<T> {
  public function __construct(public T $value) {}
  public function get(): T {
    return $this->value;
  }
  public function set(T $value): void {
    $this->value = $value;
  }
}

class C {}

function test2(bool $b1, bool $b2, C $c1, C $c2, C $c3): void {
  $r = new Ref(null);
  if ($b1) {
    $r->set($c1);
  }
  if ($b2) {
    $r->set($c2);
  }
  if ($r->get() !== null) {
    $r->set($c3);
  }
}
