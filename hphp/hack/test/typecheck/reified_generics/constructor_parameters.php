<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class A {
  public function __construct(public int $i, public string $s) {}
}

function f<<<__Newable>> reify T as A>(T $param): void {
  hh_show($param);
  $local = new T("wrong", "right");
  hh_show($local);
}
