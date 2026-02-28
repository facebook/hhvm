<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class C<reify T> {
  public function f(): void {
    new static();
  }
}
