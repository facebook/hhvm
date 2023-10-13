<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cov<+T> {
  public function __construct(private T $item) { }
    public function get(): T { return $this->item; }

  // Now some fake setters
  public function set1<Tu>(Tu $x):void where Tu as T {
    $this->item = $x;
  }

  public function set2<Tu>(Tu $x):void where T super Tu {
    $this->item = $x;
  }

  public function set3<Tu>(Tu $x):void where T = Tu {
    $this->item = $x;
  }

  public function set4(int $x):void where T super int {
    $this->item = $x;
  }
}

function setcov(Cov<mixed> $c):void {
  // All of these satisfy the constraints
  $c->set1(1);
  $c->set2(2);
  $c->set3(3);
  $c->set4(4);
}

function expectString(string $s):void { }
function breakit():void {
  $x = new Cov("a string");
  setcov($x);
  expectString($x->get());
}
