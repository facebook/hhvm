<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C<reify Tc> {}

function test(): void {
  3 as C<_>;
  3 as C<C<_>>;
}
