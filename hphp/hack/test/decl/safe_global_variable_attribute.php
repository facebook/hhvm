<?hh // strict

class B {
  public static int $static_property = 0;
  <<__SafeForGlobalAccessCheck>> public static int $static_property_safe = 0;
}

class C {
  private static int $static_property = 0;
  <<__SafeForGlobalAccessCheck>> private static int $static_property_safe = 0;
  public string $instanceProperty = "";

  public function foo(): void {
    self::$static_property = 1;
    self::$static_property_safe = 1;
    B::$static_property = 1;
    B::$static_property_safe = 1;
  }
}
