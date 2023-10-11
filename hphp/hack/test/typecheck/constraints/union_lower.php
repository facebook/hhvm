<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function bar(): void {}
}
class B1<T super int> {
  public function foo(num $n): T where T super float {
    return $n;
  }
}
