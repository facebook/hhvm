<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public static function Bar(mixed $i): int;
}
interface J {
  public static function Bar(string $s): arraykey;
}

function ExpectInt(int $a): void {}

class G<Tg as I as J> {
  public function AnotherBoo(classname<Tg> $x): void {
    ExpectInt($x::Bar('c'));
  }
}
