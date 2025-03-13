<?hh
<<__SimpliHack(DeriveGetters::class, "and setters")>>
 //   ^ at-caret
class SomeClass {
  public int $one;
  public string $word;
  public function foo(): void {}
}

class DeriveGetters {
  public static function onClass(string $action): string {
    return DeriveGetters::prompt($action);
  }

  public static function prompt(string $action): string {
    return "generate getters ".$action." for all fields";
  }
}
