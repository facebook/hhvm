<?hh

class C {
  public $nonStaticInheritedProp;

  public function nonStaticInheritedMethod() {
    echo __METHOD__, ' of ', get_called_class(), "\n";
    echo 'this: ', isset($this) ? 'defined' : 'undefined', "\n";
  }
}

trait Tr {
  require extends C;

  public $nonStaticTraitProp;

  public function nonStaticTraitMethod() {
    echo __METHOD__, ' of ', get_called_class(), "\n";
    echo 'this: ', isset($this) ? 'defined' : 'undefined', "\n";
  }
  public static function staticTraitMethod() {

    echo __METHOD__, "\n";
  }

  abstract public function nonStaticAbstract();
}

abstract final class Utils extends C {
  use Tr;

  public static function staticMethod() {
    echo __METHOD__, "\n";
    static::protStaticMethod();
  }

  private static function privStaticMethod() {
    echo __METHOD__, ' ', self::$prop, "\n";
  }

  protected static function protStaticMethod() {
    echo __METHOD__, "\n";
    self::privStaticMethod();
  }

  private static $prop = __CLASS__.'::prop';
}

function main() {
  Utils::staticMethod();
  Utils::staticTraitMethod();
  Utils::nonStaticInheritedMethod(); // works because $this isn't accessed
  Utils::nonStaticTraitMethod(); // works because $this isn't accessed

  $rc = new ReflectionClass(Utils::class);
  echo 'ReflectionClass::isAbstract', ' => ';
  var_dump($rc->isAbstract());
  echo 'ReflectionClass::isFinal', ' => ';
  var_dump($rc->isFinal());
  echo 'ReflectionClass::isInstantiable', ' => ';
  var_dump($rc->isInstantiable());
}

main();
echo 'Done', "\n";
