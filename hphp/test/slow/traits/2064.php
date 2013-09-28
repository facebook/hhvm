<?php

trait T {
  public function goo() {
    return get_called_class();
  }
  public function foo() {
     return self::goo();
  }
}
class A {
 use T;
 }
var_dump(A::goo());
var_dump(A::foo());
