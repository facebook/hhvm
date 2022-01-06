<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// No SDT
class NoSDTC {}

<<__SupportDynamicType>>
class B { }
<<__SupportDynamicType>>
class C extends B {
  public function bar():void { }
}
<<__SupportDynamicType>>
class Box<T as B> {
  public function __construct(public ~T $item) { }
}


function get():~int {
  return 3;
}

function return_pair_direct():~(int,NoSDTC) {
  return tuple(get(), new NoSDTC());
}

function return_pair():~(int,NoSDTC) {
  $x = tuple(get(), new NoSDTC());
  return $x;
}

function test_constraint_bad(Box<B> $b):~Box<C> {
  // This is dynamic-aware subtype but not
  // an ordinary subtype because we could lose errors.
  // e.g. consider $b->item->bar() which produces an error on Box<B>
  // but not on ~Box<C>
  return $b;
}
