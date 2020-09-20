<?hh
class C {  public function foo() { echo "C::foo\n"; var_dump(static::class); }
  public static function bar() { echo "C::bar\n"; var_dump(static::class); }
}
class D extends C {
  public function foo() { echo "D::foo\n"; var_dump(static::class); }
  public static function bar() { echo "D::bar\n"; var_dump(static::class); }
}
class E {
  public function foo() { echo "E::foo\n"; var_dump(static::class); }
  public static function bar() { echo "E::bar\n"; var_dump(static::class);
  }
}


<<__EntryPoint>> function main(): void {
  call_user_func(varray[new C(), 'foo']);
  call_user_func(varray[new D(), 'foo']);
  call_user_func(varray[new E(), 'foo']);

  call_user_func('C::bar');
  call_user_func('D::bar');
  call_user_func('E::bar');
}
