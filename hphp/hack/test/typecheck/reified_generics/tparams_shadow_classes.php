<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Ta {}

function f1<TA>(): void {}
function f2<Ta>(): void {}

class C1<TA> {}
class C2<Ta> {}

class CM {
  public function m1<TA>(): void {}
  public function m2<Ta>(): void {}
}

type T1<TA> = int;
type T2<Ta> = int;
