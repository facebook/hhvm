<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Ref<T> {
  public function __construct(public T $value) {}
}

function test(dict<string, int> $d): void {
  $r = new Ref($d);
  $r->value['pi'] = 3.14;
}
