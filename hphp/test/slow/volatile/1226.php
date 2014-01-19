<?php

if (class_exists('B')) {
  class A extends B {
    public function f() {
      var_dump('A');
    }
  }
}
var_dump(class_exists('C'));
class B extends C {
  public function f() {
    var_dump('B');
  }
}
class C {
}
if (class_exists('A')) {
  $obj = new A;
  $obj->f();
}
 else {
  var_dump('correct');
}
