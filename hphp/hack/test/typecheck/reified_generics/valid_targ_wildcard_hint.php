<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<reify Tc> {}

class D extends C<_> {}
class E extends C<C<_>> {}

function test(C<C<_>> $c): C<_> {
  return $c;
}
