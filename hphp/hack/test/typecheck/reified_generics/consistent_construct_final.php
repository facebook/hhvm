<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class C<reify T> {
  public function f(): void {
    new static();
  }
}
