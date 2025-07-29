<?hh

<<__EntryPoint>>
function main(): void {
  // Get the autoloader to initialize
  HH\autoload_is_native();

  $text = '<?hh
  type myType = shape("key1" => dict<string, int>, "key2" => string, FAKE_CLASS::FAKE_CONST => string);
  type testType = shape(GaryTestClass::TEST_STRING => int);

    class FooBar {
      const type fooType = shape("k1" => int, BAD_CLS::BAD_CONST => string);
      const type clsTest = shape(GaryTestClass::TEST_STRING => int);
      
      public static async function genFoo(string $_, int $a): Awaitable<int> {
        return $a;
      }
      
      public function defaultVal(string $a = "foo"): string { 
        return $a;
      }
      
      private static function hidden(): void {}
    }

    interface IImplementable {
      public function fromInterface(): void;
    }

    class Base {
      public function fromBase(): void {}
    }

    class Derived extends Base implements IImplementable {
      <<__Override>>
      public function fromInterface(): void {
        echo "void\n";
      }
    }
    
    enum class TypeArgs: IImplementable {}

    abstract class HasManyConsts {
      const type TArgs as TypeArgs;
      const type TKey = string;
      const int MY_IDX = 1;

      public function doesNothing(int $a): void {}
    }

    interface StackLike<T> {}
    class VecLike<T> implements StackLike<T>, IImplementable {
      public function __construct() {
        return parent::__construct();
      }
    }
  ';

    $instance = HH\FileDecls::parseText($text);

    echo $instance->getPublicApiForClass('FooBar');
    echo $instance->getPublicApiForClass('Derived');
    echo $instance->getPublicApiForClass('HasManyConsts');
    echo $instance->getPublicApiForClass('VecLike');
}
