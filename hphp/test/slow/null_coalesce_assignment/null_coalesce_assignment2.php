<?hh

class C {
  public static ?int $foo = null;
}

<<__EntryPoint>> function test(): void {
  C::$foo ??= 1;
  var_dump(C::$foo);
  C::$foo ??= 'foo';
  var_dump(C::$foo);
}
