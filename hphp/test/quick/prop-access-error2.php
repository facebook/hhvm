<?php

class A {
  protected $x = 1;
}

class B extends A {
  protected $x = 2;
}

class C extends A {
  public function test($obj) {
    var_dump($obj->x);
  }
}

function main() {
  $b = new B;
  $c = new C;
  $c->test($b);
}

main();
