<?hh

// Simplified version of the real BypassVisibility
class BypassVisibility {

  public static function invokeInstanceMethod(
    mixed $object,
    string $method,
    mixed ...$args
  ): void {}

  public static function invokeStaticMethod(
    classname<mixed> $classname,
    string $method,
    mixed ...$args
  ): void {}

}
