<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type T;
}

function f(C::T $_): void {}
