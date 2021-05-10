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

function gen_string_dyn() : dynamic {
  return "hello";
}

function expect_int(int $x) : void {}

<<__EntryPoint>>
function main() : void {
  $r = new Ref(42);          // (1.)
  $r->set(gen_string_dyn()); // (2.)
  expect_int($r->get());
}
