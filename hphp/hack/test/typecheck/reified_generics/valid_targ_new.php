<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<reify Tc> {}

function g<T>(): void {
  new C<T>();
}
