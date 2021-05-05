<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Ref<T> {
  public function __construct(private T $x) {}
  public function get() : T {
    return $this->x;
  }
  public function set(T $x): void {
    $this->x = $x;
  }
}

function expect_int(int $x) : void { }

function update_at_dyn(dynamic $o, string $s) : void {
  $o->set($s);
}

<<__EntryPoint>>
function main() : void {
  $r = new Ref(42);           // (3.)
  update_at_dyn($r, "hello"); // (4.)
  expect_int($r->get());      // (5.)
}
