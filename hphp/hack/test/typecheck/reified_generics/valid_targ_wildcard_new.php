<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<reify Tc> {}

function test(): void {
  new C<_>();
  new C<C<_>>();
}
