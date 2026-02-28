<?hh

final class RenameStaticMethClass {

  public static function static_test_method(): int {
    return 1;
  }

  // Find-refs should be able to find all these uses of static_test_method
  // When the method is wrapped in apostrophes, the capture should be only
  // the method name
  public static function other_method(): int {
    self::static_test_method();
    RenameStaticMethClass::static_test_method();
    self::static_test_method<>;
    RenameStaticMethClass::static_test_method<>;
    return 0;
  }
}
