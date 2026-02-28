<?hh
  /**
   * THIS FILE IS @generated; DO NOT EDIT IT
   * To regenerate this file, run
   *
   *   buck run //hphp/hack/test:gen_case_type_tests
   **/

  <<file: __EnableUnstableFeatures('case_types')>>

  

  abstract class BaseCheck {
    abstract const type T;
    abstract const string NAME;
    abstract protected static function values(): vec<this::T>;
    abstract protected static function funcParam(this::T $c): void;
    abstract protected static function funcReturn(mixed $c): this::T;
    abstract protected static function funcGenericParam<Tx as this::T>(Tx $c): void;
    abstract protected static function funcGenericReturn<Tx as this::T>(mixed $c): Tx;
    abstract protected static function propertyCheck(this::T $val): void;

    public static function run(): void {
      foreach(static::values() as $val) {
        // Param Checks
        static::funcParam($val);
        static::funcGenericParam($val);

        // Return Checks
        static::funcReturn($val);
        static::funcGenericReturn($val);

        // Property Checks
        static::propertyCheck($val);
      }
      echo (static::NAME .' Ok' . PHP_EOL);
    }
  }

  case type CT0 = null|noreturn;

  
class CheckCT0<T as CT0> extends BaseCheck {
  const type T = CT0;
  const string NAME = 'CT0';

  <<__LateInit>>
  private CT0 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT0 $c): void {}

  protected static function funcReturn(mixed $c): CT0 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT0>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT0>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT0 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT0> {
    return vec[null];
  }
}

  <<__EntryPoint>>
  function main(): void {
    CheckCT0::run();
  }