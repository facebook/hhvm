<?hh // strict

class A {
  public static function static_class(): void {
    echo static::class, "\n";
  }
}

class B extends A {
  public static function stuff(): void {
    echo self::class, "\n";
    echo parent::class, "\n";
  }
}

function test(): void {
  echo A::class, "\n";
  echo B::class, "\n";
  echo Awaitable::class, "\n";
  A::static_class();
  B::static_class();
  B::stuff();
}
