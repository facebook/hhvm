<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

function f<T as A>(class<T> $a): void {
  new $a();
}
