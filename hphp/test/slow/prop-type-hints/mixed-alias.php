<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type FakeType<T> = mixed;

class C {
  private FakeType<string> $p1;
  private FakeType<string> $p2;

  private static FakeType<string> $s1;

  public function __construct() {
    $this->p1 = 'abc';
    $this->p2 = &$this->p1;
    $this->p1 = 'def';
    C::$s1 = &$this->p1;
  }

  public function blah() {
    $s = $this->p2;
    var_dump($s);
  }
}

function main() {
  $c = new C();
  $c->blah();
}
main();
