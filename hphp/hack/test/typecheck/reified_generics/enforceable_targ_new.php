<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<Te> {}
class Reified<reify Tr> {}

class C<<<__Enforceable>> Tc> {}

function test(): void {
  new C<Erased<int>>();
  new C<Reified<int>>();
  new C<Reified<Erased<int>>>();
}
