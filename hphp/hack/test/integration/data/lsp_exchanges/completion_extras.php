<?hh

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
