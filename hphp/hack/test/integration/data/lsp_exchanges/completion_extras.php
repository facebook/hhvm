<?hh  //strict

/** :ab:cd:text docblock */
final class :ab:cd:text implements XHPChild {
  attribute string color, int width;
}

/** :ab:cd:alpha docblock */
final class :ab:cd:alpha implements XHPChild {
  attribute string name;
}

enum Elsa: string {
  Alonso = "hello";
  Bard = "world";
}

final class DeprecatedClass {
  public static function getName(): void {}
  public static function __getLoader(): void {}
  public static function getAttributes_DO_NOT_USE(): void {}
  public static function test_do_not_use(): void {}
}
