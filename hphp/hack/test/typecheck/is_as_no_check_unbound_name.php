<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A<T> {}

function f(): void {
  // This will not produce a 4195 error now
  3 as B<int>;
}
