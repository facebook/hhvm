<?hh
class C {
  public function foo() { echo "C::foo\n"; var_dump(static::class); }
  public static function bar() { echo "C::bar\n"; var_dump(static::class); }
}
class D extends C {
  public function foo() { echo "D::foo\n"; var_dump(static::class); }
  public static function bar() { echo "D::bar\n"; var_dump(static::class); }
}
class E {
  public function foo() { echo "E::foo\n"; var_dump(static::class); }
  public static function bar() { echo "E::bar\n"; var_dump(static::class); }
}


<<__EntryPoint>> function main(): void {
  $c = new C();
  $d = new D();
  $e = new E();

  call_user_func(array($c, 'foo'));
  call_user_func(array($c, 'C::foo'));
  call_user_func(array($c, 'D::foo')); // warns and bails returning null
  call_user_func(array($c, 'E::foo')); // warns and bails returning null

  call_user_func(array('C', 'bar'));
  call_user_func(array('C', 'C::bar'));
  call_user_func(array('C', 'D::bar')); // warns and bails returning null
  call_user_func(array('C', 'E::bar')); // warns and bails returning null

  call_user_func(array($d, 'foo'));
  call_user_func(array($d, 'C::foo'));
  call_user_func(array($d, 'D::foo'));
  call_user_func(array($d, 'E::foo')); // warns and bails returning null

  call_user_func(array('D', 'bar'));
  call_user_func(array('D', 'C::bar'));
  call_user_func(array('D', 'D::bar'));
  call_user_func(array('D', 'E::bar')); // warns and bails returning null

  call_user_func(array($e, 'foo'));
  call_user_func(array($e, 'C::foo'));
  call_user_func(array($e, 'D::foo'));
  call_user_func(array($e, 'E::foo')); // warns and bails returning null

  call_user_func(array('E', 'bar'));
  call_user_func(array('E', 'C::bar'));
  call_user_func(array('E', 'D::bar'));
  call_user_func(array('E', 'E::bar')); // warns and bails returning null
}
