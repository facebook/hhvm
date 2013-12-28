<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1 {
  public function baz();
}

class Super {
  protected function foo() {
    echo "Super::foo\n";
  }
}

trait T1 {
  require extends Super;

  require implements I1;

  public function bar() {
    return $this->foo();
  }
}

class C implements I1 {
  use T1;

  public function baz() {}
}

function main() {
  $c = new C();
  $c->bar();
}
main();
