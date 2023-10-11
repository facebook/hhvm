<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C<reify Tc> {}
function f<reify Tf>(): void {}

function test(): void {
  f<_>();
  f<C<_>>();
}
