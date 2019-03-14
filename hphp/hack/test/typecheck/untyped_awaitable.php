<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function bar(): int {
    return 3;
  }
}
/* HH_FIXME[4101] */
async function foo(C $c): Awaitable {
  return $c->bar();
}
