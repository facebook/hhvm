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
  $obj = new C;
  call_user_func(vec[$obj, 'foo']);
  call_user_func(vec[$obj, 'C::foo']);
  call_user_func(vec[$obj, 'D::foo']); // warns and bails returning null
  call_user_func(vec[$obj, 'E::foo']); // warns and bails returning null

  call_user_func(vec[$obj, 'bar']);
  call_user_func(vec[$obj, 'C::bar']);
  call_user_func(vec[$obj, 'D::bar']); // warns and bails returning null
  call_user_func(vec[$obj, 'E::bar']); // warns and bails returning null

  $obj = new D;
  call_user_func(vec[$obj, 'foo']);
  call_user_func(vec[$obj, 'C::foo']);
  call_user_func(vec[$obj, 'D::foo']);
  call_user_func(vec[$obj, 'E::foo']); // warns and bails returning null

  call_user_func(vec[$obj, 'bar']);
  call_user_func(vec[$obj, 'C::bar']);
  call_user_func(vec[$obj, 'D::bar']);
  call_user_func(vec[$obj, 'E::bar']); // warns and bails returning null

  $obj = new E;
  call_user_func(vec[$obj, 'foo']);
  call_user_func(vec[$obj, 'C::foo']);
  call_user_func(vec[$obj, 'D::foo']);
  call_user_func(vec[$obj, 'E::foo']); // warns and bails returning null

  call_user_func(vec[$obj, 'bar']);
  call_user_func(vec[$obj, 'C::bar']);
  call_user_func(vec[$obj, 'D::bar']);
  call_user_func(vec[$obj, 'E::bar']); // warns and bails returning null
}
