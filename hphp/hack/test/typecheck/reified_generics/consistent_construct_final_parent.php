<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class A {
  public function f(): void {
    new static();
  }
}

final class B<reify T> extends A {}
