<?php

class B {
}
class A extends B {
  static function make() {
    $b = new parent();
    $a = new self();
  }
}
if (false) {
 class A {
}
;
}
A::make();
