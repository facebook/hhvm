<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}

async function gen_c(): Awaitable<C> {
  return new C();
}

async function testit(): Awaitable<void> {
  $c = await gen_c();
  $c->foo();
}
