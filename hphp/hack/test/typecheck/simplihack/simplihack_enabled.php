<?hh
<<file: __EnableUnstableFeatures('simpli_hack')>>
<<__SimpliHack(DeriveGetters::onClass(), '3aab93f88e7f141f09f6d0f8dd40e097')>>
class SomeOtherClass {
  public function foo(): void {}
}

class DeriveGetters {
  public static function onClass(): string {
    return "generate getters and setters for all fields";
  }
}
