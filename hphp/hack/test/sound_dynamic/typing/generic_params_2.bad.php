<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class Box<T> {
  public function __construct(private T $x) {}
  public function get() : T { return $this->x; }
  public function set(T $x) : void { $this->x = $x; }
  public function call_set() : void {
    $this->set("1" as dynamic);
  }
}

function expect_int(int $i) : void {}

<<__EntryPoint>>
function f() : void {
  $b = new Box<int>(1);
  $d = "1" as dynamic;
  $b->call_set();
  expect_int($b->get());
}
