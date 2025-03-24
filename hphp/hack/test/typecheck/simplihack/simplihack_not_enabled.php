<?hh
<<__SimpliHack(DeriveGetters::class)>>
class SomeOtherClass {
  public function foo(): void {}
}

class DeriveGetters {
  public static function onClass(): string {
    return "generate getters and setters for all fields";
  }
}
