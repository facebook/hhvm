<?php
class C {
  public function baz($z) {
    $x = 1;
    return $z = call_user_func(array($this,'foo'), $z);
  }
  public function bar($z) {
    $x = 1;
    return $z = call_user_func(array($this,'baz'), $z);
  }
  public function foo($z) {
    $x = 1;
    return $z = call_user_func(array($this,'bar'), $z);
  }
}
function bar() {
  $obj = new C;
  $obj->foo(123);
}
bar();
