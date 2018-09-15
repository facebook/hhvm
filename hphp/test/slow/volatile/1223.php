<?php

class B {
}
class A extends B {
  static function make() {
    $b = new parent();
    $a = new self();
  }
}

<<__EntryPoint>>
function main_1223() {
if (false) {
 class A {
}
;
}
A::make();
}
