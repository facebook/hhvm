<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<<<__Soft>> reify T> {}

function f(): void {
  3 as C<_>;
  3 as C<int>;
}
