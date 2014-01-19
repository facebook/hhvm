<?php

class B {
  public static function bar() {
    echo "B\n";
  }
}

class D extends B {
  public static function bar() {
    echo "D\n";
  }
}

trait Yeah {
  public function foo() {
    // Bug #2339698.  Parent was skipping one.
    parent::bar();
  }
}

class C extends D {
  use Yeah;
}

function foo() {
  $k = new C();
  $k->foo();
}

foo();
