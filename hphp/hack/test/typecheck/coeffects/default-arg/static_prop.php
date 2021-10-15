<?hh

abstract class AC {
  public static Set<int> $s = Set {};

  public static function accessStaticPropWithDefaultCtx(
    Set<int> $x = static::$s // OK
  ): void {}

  public static function accessStaticPropWithPureCtx(
    Set<int> $x = static::$s  // ERROR
  )[]: void {}

  public function accessStaticPropWithDefaultCtxNonStatic(
    Set<int> $x = self::$s // OK
  ): void {}

  public function accessStaticPropWithPureCtxNonStatic(
    Set<int> $x = self::$s  // ERROR
  )[]: void {}
}

function accessStaticPropWithDefaultCtx(
  Set<int> $x = AC::$s  // OK
): void {}

function accessStaticPropWithPureCtx(
  Set<int> $x = AC::$s  // ERROR
)[]: void {}

function accessStaticPropWithAccessGlobalsCtx(
  Set<int> $x = AC::$s  // OK
)[globals]: void {}

abstract class AC2 extends AC {
  public static function accessStaticPropWithAccessGlobalsCtx(
    Set<int> $x = AC::$s  // OK
  )[globals]: void {}

  public function accessStaticPropWithAccessGlobalsCtxNonStatic(
    Set<int> $x = parent::$s  // OK
  )[globals]: void {}
}
