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
  include '1223.inc';
;
}
A::make();
}
