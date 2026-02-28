<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T as ?int> {
  public function foo(?arraykey $ak): void {}
  public function bar(T $x): void {
    $this->foo($x);
  }
}
