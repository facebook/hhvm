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
  $obj = new C;
  call_user_func(array($obj, 'foo'));
  call_user_func(array($obj, 'C::foo'));
  call_user_func(array($obj, 'D::foo')); // warns and bails returning null
  call_user_func(array($obj, 'E::foo')); // warns and bails returning null

  call_user_func(array($obj, 'bar'));
  call_user_func(array($obj, 'C::bar'));
  call_user_func(array($obj, 'D::bar')); // warns and bails returning null
  call_user_func(array($obj, 'E::bar')); // warns and bails returning null

  $obj = new D;
  call_user_func(array($obj, 'foo'));
  call_user_func(array($obj, 'C::foo'));
  call_user_func(array($obj, 'D::foo'));
  call_user_func(array($obj, 'E::foo')); // warns and bails returning null

  call_user_func(array($obj, 'bar'));
  call_user_func(array($obj, 'C::bar'));
  call_user_func(array($obj, 'D::bar'));
  call_user_func(array($obj, 'E::bar')); // warns and bails returning null

  $obj = new E;
  call_user_func(array($obj, 'foo'));
  call_user_func(array($obj, 'C::foo'));
  call_user_func(array($obj, 'D::foo'));
  call_user_func(array($obj, 'E::foo')); // warns and bails returning null

  call_user_func(array($obj, 'bar'));
  call_user_func(array($obj, 'C::bar'));
  call_user_func(array($obj, 'D::bar'));
  call_user_func(array($obj, 'E::bar')); // warns and bails returning null
}
main();

