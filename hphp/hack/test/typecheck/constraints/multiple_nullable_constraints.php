<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function foo(): void;
}
interface J {
  public function bar(): void;
}

class B<T as ?I as ?J> {
  public function testit(T $obj): void where T as nonnull {
    $obj->foo();
    $obj->bar();
  }
}
