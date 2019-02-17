<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<Te> {}
class Reified<reify Tr> {}

class C {
  public function m<<<__Enforceable>> Tf>(): void {}
}

function test(): void {
  $c = new C();
  $c->m<Erased<int>>();
  $c->m<Reified<int>>();
  $c->m<Reified<Erased<int>>>();
}
