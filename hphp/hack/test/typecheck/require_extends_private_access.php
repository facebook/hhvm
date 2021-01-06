<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  require extends B;
}

class B {
  private function privatef():void { }
  public function publicf():void { }
  public function test1(I $i):void {
    $i->privatef();
    $i->publicf();
  }
  public function test2(B $b, C $c):void {
    $b->privatef();
    $c->privatef();
    $c->publicf();
    if ($b is I) {
      $b->privatef();
    }
    if ($this is I) {
      $this->privatef();
    }
  }
}

class C extends B {
  public function test3(I $i):void {
    $i->privatef();
  }
  public function test4(B $b):void {
    $b->privatef();
  }
}
