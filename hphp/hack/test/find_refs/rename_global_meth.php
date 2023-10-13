<?hh

function global_test_method(): int {
  return 1;
}

class RenameGlobalMethClass {

  // Find-refs should be able to find all these uses of global_test_method
  // When the method is wrapped in apostrophes, the capture should be only
  // the method name
  public static function other_method(): int {
    global_test_method();
    $bar = global_test_method<>;
    return 0;
  }
}
