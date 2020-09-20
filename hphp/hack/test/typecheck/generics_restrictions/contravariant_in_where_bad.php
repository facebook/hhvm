<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Contra<-T> {
  public function __construct(private T $item) { }
    public function set(T $x): void { $this->item = $x; }

  // Now some fake getters
  public function get1<Tu>():Tu where T as Tu {
    return $this->item;
  }

  public function get2<Tu>():Tu where Tu super T {
    return $this->item;
  }

  public function get3<Tu>():Tu where T = Tu {
    return $this->item;
  }

  public function get4():int where T as int {
    return $this->item;
  }
}

function expectInt(int $s):void { }

function getcontra(Contra<int> $c):void {
  // All of these satisfy the constraints
  expectInt($c->get1());
  expectInt($c->get2());
  expectInt($c->get3());
  expectInt($c->get4());
}

function breakit():void {
  $x = new Contra<arraykey>("a string");
  getcontra($x);
}
