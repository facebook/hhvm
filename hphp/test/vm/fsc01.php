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
  public static function f2() {
    var_dump(get_called_class());
    echo "\n";
  }
}
class C extends B {
  /**
   * Under Zend, forward_static_call(array($obj, 'f1')) will pass in $obj
   * as the current instance ($obj is of class B) but it sets the late bound
   * class to C. This violates an invariant stated in Zend PHP's documentation
   * at http://us.php.net/lsb which states "In non-static contexts, the called
   * class will be the class of the object instance." For these cases, HipHop
   * reconciles this issue by setting the late bound class to the class of the
   * current instance if there is one.
   */
  public function g() {
    $obj = new B;
    $obj->f1(); // B B
    B::f1(); // C C
    forward_static_call(array($obj, 'f1')); // B B (Zend PHP 5.3 outputs C B)
    forward_static_call(array('B', 'f1')); // C C
    echo "***************\n";
    $obj->f2(); // B
    B::f2(); // B
    forward_static_call(array($obj, 'f2')); // C
    forward_static_call(array('B', 'f2')); // C
  }
}
$obj = new C;
$obj->g();
