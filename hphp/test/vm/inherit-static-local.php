<?php

class C {
  public static function foo() {
    static $x = 0;
    ++$x;
    var_dump($x);
  }
}
class D extends C {
  public static function callByParent() {
    parent::foo();
  }
}

C::foo();
C::foo();
C::foo();
D::foo();
D::foo();

D::callByParent();
D::callByParent();
