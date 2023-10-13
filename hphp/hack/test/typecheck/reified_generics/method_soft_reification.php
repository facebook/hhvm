<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function erased<T>(): void {}
  public function softReified<<<__Soft>> reify T>(): void {}
  public function reified<reify T>(): void {}
}

function call_keywordCheck(): void {
  $c = new C();

  $c->erased();
  $c->erased<int>();

  $c->softReified(); // bad
  $c->softReified<int>();

  $c->reified(); // bad
  $c->reified<int>();
}

function reification_test<
  Terase,
  <<__Soft>> reify Tsoft,
  reify Thard
>(): void {
  $c = new C();

  $c->erased<Terase>();
  $c->erased<Tsoft>();
  $c->erased<Thard>();

  $c->softReified<Terase>();
  $c->softReified<Tsoft>();
  $c->softReified<Thard>();

  $c->reified<Terase>();
  $c->reified<Tsoft>();
  $c->reified<Thard>();
}
