<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function f(): void;
}

class A implements I {
  public function f(): void {}
  public function g(): void {}
}

class B implements I {
  public function f(): void {}
  public function h(): void {}
}

function f(): void {}

class C extends XHPTest implements XHPChild {
  attribute ?I x = null;

  public function test(): void {
    if ($this->:x !== null) {
      f();
      if ($this->:x is A) {
        $this->:x->g();
      } else if ($this->:x is B) {
        $this->:x->h();
      } else {
        $this->:x?->f();
      }
    }
  }
}
