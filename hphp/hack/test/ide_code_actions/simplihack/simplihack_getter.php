<?hh
<<__SimpliHack(DeriveGetters::class)>>
 //   ^ at-caret
class SomeClass {
  public int $one;
  public string $word;
  public function foo(): void {}
}

class DeriveGetters {
  public static function onClass(): string {
    return "generate getters and setters for all fields"
  }
}
