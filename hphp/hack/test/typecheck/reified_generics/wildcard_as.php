<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A<reify T> {}

function f(): void {
  3 as _;
  3 as A<_>;
  3 as A<A<_>>;
  3 as A<A<A<_>>>;
}
