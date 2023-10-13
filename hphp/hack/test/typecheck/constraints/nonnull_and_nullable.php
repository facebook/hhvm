<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function bar(): void {}
}
class B1<T as nonnull> {
  public function foo(T $x): void where T as ?C {
    $x->bar();
  }
}
class B2<T as ?C> {
  public function foo(T $x): void where T as nonnull {
    $x->bar();
  }
}
