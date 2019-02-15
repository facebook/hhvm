<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function erased<T>(): void {}
  public function reified<reify T>(): void {}
}

function call_keywordCheck(): void {
  $c = new C();

  $c->erased();
  $c->erased<int>();
  $c->erased<int>();

  $c->reified(); // bad
  $c->reified<int>();
  $c->reified<int>();
}
