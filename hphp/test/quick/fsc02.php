<?php
class B {
  public function f1() {
    var_dump(get_called_class());
    if ($this !== null) {
      var_dump(get_class($this));
    } else {
      var_dump(null);
    }
    echo "\n";
  }
}
class C extends B {
  public function g() {
    $obj = new B;
    forward_static_call(array('B', 'f1'));
    call_user_func(array('B', 'f1'));
  }
}
$obj = new C;
$obj->g();
