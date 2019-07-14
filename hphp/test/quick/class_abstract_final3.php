<?hh

class C {
  public function nonStaticInheritedMethod() {
    echo __METHOD__, ' of ', static::class, "\n";
    echo 'this: ', isset($this) ? 'defined' : 'undefined', "\n";
  }
}

trait Tr {
  require extends C;

  public function nonStaticTraitMethod() {
    echo __METHOD__, ' of ', static::class, "\n";
    echo 'this: ', isset($this) ? 'defined' : 'undefined', "\n";
  }
  public static function staticTraitMethod() {
    echo __METHOD__, "\n";
  }

  abstract static function noGood();
}

abstract final class Utils extends C {
  use Tr;

  public static function staticMethod() {
    echo __METHOD__, "\n";
  }
}
<<__EntryPoint>> function main_entry(): void {
main();
echo 'Done', "\n";
}
