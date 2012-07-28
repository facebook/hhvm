<?php

class my_base {
  public function __call($f, $a) {
    echo "my_base::__call: " . $f . "(" . $a[0] . "," . $a[1] . ")\n";
  }
  static public function __callStatic($f, $a) {
    echo "my_base::__callStatic: " . $f . "(" . $a[0] . "," . $a[1] . ")\n";
  }
  public static function test() {
    echo "my_base::test\n";
  }
}
class my_class1 extends my_base {
  public function __call($f, $a) {
    echo "my_class1::__call: " . $f . "(" . $a[0] . "," . $a[1] . ")\n";
  }
  static public function __callStatic($f, $a) {
    echo "my_class1::__callStatic: " . $f . "(" . $a[0] . "," . $a[1] . ")\n";
  }
  public function test_self() {
    call_user_func(array('self', 'nonexistent'), "a", "b");
  }
  public function test_parent() {
    call_user_func(array('parent', 'nonexistent'), "a", "b");
  }
}

class my_class2 extends my_base {}


function main() {
  $obj1 = new my_class1;
  call_user_func(array($obj1, 'nonexistent'), "1", "2");
  call_user_func(array('my_class1', 'nonexistent'), "1", "2");
  call_user_func('my_class1::nonexistent', "1", "2");
  call_user_func_array('my_class1::nonexistent', array("1", "2"));
  call_user_func(array($obj1, '__call_static'), "1", "2");
  call_user_func(array('my_class1', '__call_static'), "1", "2");
  call_user_func(array('my_class1', 'test'));
  echo "test_self:   "; $obj1->test_self();
  echo "test_parent: "; $obj1->test_parent();

  $obj2 = new my_class2;
  call_user_func(array($obj2, 'nonexistent'), "1", "2");
  call_user_func(array('my_class2', 'nonexistent'), "1", "2");
  call_user_func('my_class2::nonexistent', "1", "2");
}
main();

