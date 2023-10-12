<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function Foo(int $i): void;
}
interface J {
  public function Bar(string $s): arraykey;
}
interface K extends J {
  public function Bar(string $s): int;
  public function Another(float $f): void;
}

function ExpectInt(int $a): void {}

// Just check that redundant constraints don't matter
// On invoking we pick the more specific one
class G<Tg as K as J> {
  public function CallJBar(J $x): void {
    $x->Bar('b');
  }
  public function CallKBar(K $x): int {
    return $x->Bar('a');
  }
  public function AnotherBoo(Tg $x): void {
    $this->CallJBar($x);
    ExpectInt($this->CallKBar($x));
    ExpectInt($x->Bar('c'));
  }
}
