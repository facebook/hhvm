<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C<T> {}

<<__SupportDynamicType>>
class D {
  public function bar(C<int> $x) : int { return 1; }
  public function foo(C<int> $x) : int { return $this->bar($x); }
}
