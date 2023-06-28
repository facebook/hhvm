<?hh
class C {
  public function foo() :mixed{ echo "C::foo\n"; var_dump(static::class); }
  public static function bar() :mixed{ echo "C::bar\n"; var_dump(static::class); }
}
class D extends C {
  public function foo() :mixed{ echo "D::foo\n"; var_dump(static::class); }
  public static function bar() :mixed{ echo "D::bar\n"; var_dump(static::class); }
}
class E {
  public function foo() :mixed{ echo "E::foo\n"; var_dump(static::class); }
  public static function bar() :mixed{ echo "E::bar\n"; var_dump(static::class); }
}


<<__EntryPoint>> function main(): void {
  $c = new C();
  $d = new D();
  $e = new E();

  call_user_func(varray[$c, 'foo']);
  call_user_func(varray[$c, 'C::foo']);
  call_user_func(varray[$c, 'D::foo']); // warns and bails returning null
  call_user_func(varray[$c, 'E::foo']); // warns and bails returning null

  call_user_func(varray['C', 'bar']);
  call_user_func(varray['C', 'C::bar']);
  call_user_func(varray['C', 'D::bar']); // warns and bails returning null
  call_user_func(varray['C', 'E::bar']); // warns and bails returning null

  call_user_func(varray[$d, 'foo']);
  call_user_func(varray[$d, 'C::foo']);
  call_user_func(varray[$d, 'D::foo']);
  call_user_func(varray[$d, 'E::foo']); // warns and bails returning null

  call_user_func(varray['D', 'bar']);
  call_user_func(varray['D', 'C::bar']);
  call_user_func(varray['D', 'D::bar']);
  call_user_func(varray['D', 'E::bar']); // warns and bails returning null

  call_user_func(varray[$e, 'foo']);
  call_user_func(varray[$e, 'C::foo']);
  call_user_func(varray[$e, 'D::foo']);
  call_user_func(varray[$e, 'E::foo']); // warns and bails returning null

  call_user_func(varray['E', 'bar']);
  call_user_func(varray['E', 'C::bar']);
  call_user_func(varray['E', 'D::bar']);
  call_user_func(varray['E', 'E::bar']); // warns and bails returning null
}
