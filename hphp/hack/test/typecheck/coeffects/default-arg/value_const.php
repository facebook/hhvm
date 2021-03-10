<?hh

class Foo {
  const int C1 = 42;

  public static function accessValueConstWithDefaultCtx(
    int $x = self::C1
  ): void {}

  public static function accessValueConstWithPureCtx(
    int $x = self::C1
  )[]: void {}

  public function accessValueConstWithDefaultCtxNonStatic(
    int $x = self::C1
  ): void {}

  public function accessValueConstWithPureCtxNonStatic(
    int $x = self::C1
  )[]: void {}
}

function accessValueConstWithDefaultCtx(
  int $x = Foo::C1
): void {}

function accessValueConstWithPureCtx(
  int $x = Foo::C1
)[]: void {}
