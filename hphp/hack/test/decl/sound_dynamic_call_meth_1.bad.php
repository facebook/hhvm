<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class A<T> {}

class C {
  <<__SupportDynamicType>>
  public function expect_A_int(A<int> $a1, A<int> $a2) : void {}
}


// handling of Sound Dynamic Callable methods defined in other classes
// is not complete; this should ideally be accepted, but it is not
<<__SupportDynamicType>>
class Foo {
  public function foo(A<int> $a) : void {
    (new C())->expect_A_int($a, new A<int>());
  }
  public function bar(A<int> $a) : void {
    (new C())->expect_A_int(new A<int>(), $a);
  }
}
