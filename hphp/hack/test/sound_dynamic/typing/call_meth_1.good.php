<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A<T> {}

class C {
  <<__SupportDynamicType>>
  public function expect_A_int(A<int> $a) : void {}
}

<<__SupportDynamicType>>
class D {
  public function expect_A_int(A<int> $a) : void {}
}

<<__SupportDynamicType>>
class Foo {
  public function foo(A<int> $a) : void {
    (new C())->expect_A_int($a);
    (new C())->expect_A_int(new A<int>());
    (new D())->expect_A_int($a);
    (new D())->expect_A_int(new A<int>());
  }
}
