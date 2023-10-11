<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function get(): this {
    return $this;
  }

  public function set(?this $_): void {}
}

function test(C $c): void {
  $d = $c->get();
  $d->set($d);
}
