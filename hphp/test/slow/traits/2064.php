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

<<__EntryPoint>>
function main_2064() {
var_dump(A::goo());
var_dump(A::foo());
}
