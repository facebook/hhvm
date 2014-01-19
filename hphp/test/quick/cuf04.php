<?php
class C {
  public function foo() { echo "C::foo\n"; var_dump(get_called_class()); }
  public static function bar() { echo "C::bar\n"; var_dump(get_called_class()); }
}
class D extends C {
  public function foo() { echo "D::foo\n"; var_dump(get_called_class()); }
  public static function bar() { echo "D::bar\n"; var_dump(get_called_class()); }
}
class E {
  public function foo() { echo "E::foo\n"; var_dump(get_called_class()); }
  public static function bar() { echo "E::bar\n"; var_dump(get_called_class()); }
}


function main() {
  call_user_func(array('C', 'foo'));
  call_user_func(array('C', 'C::foo'));
  call_user_func(array('C', 'D::foo')); // warns and bails returning null
  call_user_func(array('C', 'E::foo')); // warns and bails returning null

  call_user_func(array('C', 'bar'));
  call_user_func(array('C', 'C::bar'));
  call_user_func(array('C', 'D::bar')); // warns and bails returning null
  call_user_func(array('C', 'E::bar')); // warns and bails returning null

  call_user_func(array('D', 'foo'));
  call_user_func(array('D', 'C::foo'));
  call_user_func(array('D', 'D::foo'));
  call_user_func(array('D', 'E::foo')); // warns and bails returning null

  call_user_func(array('D', 'bar'));
  call_user_func(array('D', 'C::bar'));
  call_user_func(array('D', 'D::bar'));
  call_user_func(array('D', 'E::bar')); // warns and bails returning null

  call_user_func(array('E', 'foo'));
  call_user_func(array('E', 'C::foo'));
  call_user_func(array('E', 'D::foo'));
  call_user_func(array('E', 'E::foo')); // warns and bails returning null

  call_user_func(array('E', 'bar'));
  call_user_func(array('E', 'C::bar'));
  call_user_func(array('E', 'D::bar'));
  call_user_func(array('E', 'E::bar')); // warns and bails returning null
}
main();

