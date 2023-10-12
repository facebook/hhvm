<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function Bar(mixed $i): int;
}
interface J {
  public function Bar(string $s): arraykey;
}

function ExpectInt(int $a): void {}

class G<Tg as I as J> {
  public function AnotherBoo(Tg $x): void {
    ExpectInt($x->Bar('c'));
  }
}
