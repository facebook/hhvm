<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f<reify Tpain>(): void {}

function g<T>(): void {
  f<T>();
}
