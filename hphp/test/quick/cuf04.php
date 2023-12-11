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

  call_user_func(vec[$c, 'foo']);
  call_user_func(vec[$c, 'C::foo']);
  call_user_func(vec[$c, 'D::foo']); // warns and bails returning null
  call_user_func(vec[$c, 'E::foo']); // warns and bails returning null

  call_user_func(vec['C', 'bar']);
  call_user_func(vec['C', 'C::bar']);
  call_user_func(vec['C', 'D::bar']); // warns and bails returning null
  call_user_func(vec['C', 'E::bar']); // warns and bails returning null

  call_user_func(vec[$d, 'foo']);
  call_user_func(vec[$d, 'C::foo']);
  call_user_func(vec[$d, 'D::foo']);
  call_user_func(vec[$d, 'E::foo']); // warns and bails returning null

  call_user_func(vec['D', 'bar']);
  call_user_func(vec['D', 'C::bar']);
  call_user_func(vec['D', 'D::bar']);
  call_user_func(vec['D', 'E::bar']); // warns and bails returning null

  call_user_func(vec[$e, 'foo']);
  call_user_func(vec[$e, 'C::foo']);
  call_user_func(vec[$e, 'D::foo']);
  call_user_func(vec[$e, 'E::foo']); // warns and bails returning null

  call_user_func(vec['E', 'bar']);
  call_user_func(vec['E', 'C::bar']);
  call_user_func(vec['E', 'D::bar']);
  call_user_func(vec['E', 'E::bar']); // warns and bails returning null
}
