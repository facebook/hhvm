<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I { }
class C implements I {
  const type TP = int;
  public function get():(this::TP, this) { return tuple(3, $this); }
}
class D implements I {
  const type TP = string;
  public function get():(this::TP, this) { return tuple("A", $this); }
}

function foo<T as (I & (C|D)), TP>(T $x):TP where TP = T::TP {
  return $x->get()[0];
}
