<?php
function g($obj) {
  echo "g\n";
  $obj->yar(123);
}
class D {
  public function yar($y) {
    $x = $y;
  }
  // C++ exceptions, like stack overflow, now prevent further php code from
  // running. So this destructor should not run.
  public function __destruct() {
    g($this);
  }
}
class C {
  public function baz($z) {
    return $z = call_user_func(array($this,'foo'), $z);
  }
  public function bar($z) {
    return $z = call_user_func(array($this,'baz'), $z);
  }
  public function foo($z) {
    $guard = new D;
    return $z = call_user_func(array($this,'bar'), $z);
  }
}
function bar() {
  $obj = new C;
  $obj->foo(123);
}
bar();
