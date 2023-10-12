<?hh // strict

class C {
  public static ?int $foo = null;
}

function g(): void {
  C::$foo ??= 0;
}
