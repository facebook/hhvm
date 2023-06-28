<?hh

class C {
  public function nonStaticInheritedMethod() :mixed{
    echo __METHOD__, ' of ', static::class, "\n";
    echo 'this: ', isset($this) ? 'defined' : 'undefined', "\n";
  }
}

trait Tr {
  require extends C;

  public function nonStaticTraitMethod() :mixed{
    echo __METHOD__, ' of ', static::class, "\n";
    echo 'this: ', isset($this) ? 'defined' : 'undefined', "\n";
  }
  public static function staticTraitMethod() :mixed{
    echo __METHOD__, "\n";
  }

  abstract static function noGood():mixed;
}

abstract final class Utils extends C {
  use Tr;

  public static function staticMethod() :mixed{
    echo __METHOD__, "\n";
  }
}
<<__EntryPoint>> function main_entry(): void {
main();
echo 'Done', "\n";
}
