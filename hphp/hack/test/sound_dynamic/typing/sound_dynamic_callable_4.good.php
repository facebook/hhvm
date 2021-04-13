<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Evil<T> {}

class A<T> {}

class B extends A<int> {}

class C {
  public function expect_A(A<int> $a) : void {}
}

<<__SoundDynamicCallable>>
class Foo {
  public function foo(Evil<int> $x, B $a) : void {
    (new C())->expect_A($a);
  }

}
