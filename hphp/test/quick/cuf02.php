<?hh

class C {
  <<__DynamicallyCallable>>
  public function foo() :mixed{ echo "C::foo\n"; var_dump(static::class); }
  <<__DynamicallyCallable>>
  public static function bar() :mixed{ echo "C::bar\n"; var_dump(static::class); }
}

class D extends C {
  <<__DynamicallyCallable>>
  public function foo() :mixed{ echo "D::foo\n"; var_dump(static::class); }
  <<__DynamicallyCallable>>
  public static function bar() :mixed{ echo "D::bar\n"; var_dump(static::class); }
}

class E {
  <<__DynamicallyCallable>>
  public function foo() :mixed{ echo "E::foo\n"; var_dump(static::class); }
  <<__DynamicallyCallable>>
  public static function bar() :mixed{ echo "E::bar\n"; var_dump(static::class); }
}

<<__EntryPoint>>
function main(): void {
  call_user_func(vec[new C(), 'foo']);
  call_user_func(vec[new D(), 'foo']);
  call_user_func(vec[new E(), 'foo']);

  call_user_func('C::bar');
  call_user_func('D::bar');
  call_user_func('E::bar');
}
