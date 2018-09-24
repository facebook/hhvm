<?hh // strict

class C {
  public static ?int $foo = null;
}

function test(): void {
  C::$foo ??= 1;
  var_dump(C::$foo);
  C::$foo ??= 'foo';
  var_dump(C::$foo);
}

test();
