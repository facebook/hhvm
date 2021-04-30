<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C<T> {}

class E {}

<<__SupportDynamicType>>
class D {
  public function bar(E $x) : int { return 1; }

  public function foo(C<int> $x) : int { return $this->bar(new E()); }
}
