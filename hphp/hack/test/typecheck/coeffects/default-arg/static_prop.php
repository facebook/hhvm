<?hh

abstract class AC {
  public static Set<int> $s = Set {};

  public static function accessStaticPropWithDefaultCtx(
    Set<int> $x = static::$s
  ): void {}

  public static function accessStaticPropWithPureCtx(
    Set<int> $x = static::$s
  )[]: void {}

  public function accessStaticPropWithDefaultCtxNonStatic(
    Set<int> $x = self::$s
  ): void {}

  public function accessStaticPropWithPureCtxNonStatic(
    Set<int> $x = self::$s
  )[]: void {}
}

function accessStaticPropWithDefaultCtx(
  Set<int> $x = AC::$s
): void {}

function accessStaticPropWithPureCtx(
  Set<int> $x = AC::$s
)[]: void {}
