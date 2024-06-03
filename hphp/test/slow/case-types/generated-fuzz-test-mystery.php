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

  case type CT0 = ?bool|?bool;

  
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
    return vec[false,null,true];
  }
}
case type CT1 = float|?bool;

  
class CheckCT1<T as CT1> extends BaseCheck {
  const type T = CT1;
  const string NAME = 'CT1';

  <<__LateInit>>
  private CT1 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT1 $c): void {}

  protected static function funcReturn(mixed $c): CT1 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT1>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT1>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT1 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT1> {
    return vec[0.0,3.14,false,null,true];
  }
}
case type CT2 = string|?bool;

  
class CheckCT2<T as CT2> extends BaseCheck {
  const type T = CT2;
  const string NAME = 'CT2';

  <<__LateInit>>
  private CT2 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT2 $c): void {}

  protected static function funcReturn(mixed $c): CT2 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT2>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT2>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT2 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT2> {
    return vec['','hello world',false,null,true];
  }
}
case type CT3 = vec<mixed>|?bool;

  
class CheckCT3<T as CT3> extends BaseCheck {
  const type T = CT3;
  const string NAME = 'CT3';

  <<__LateInit>>
  private CT3 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT3 $c): void {}

  protected static function funcReturn(mixed $c): CT3 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT3>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT3>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT3 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT3> {
    return vec[false,null,true,vec[]];
  }
}
case type CT4 = ?bool|float;

  
class CheckCT4<T as CT4> extends BaseCheck {
  const type T = CT4;
  const string NAME = 'CT4';

  <<__LateInit>>
  private CT4 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT4 $c): void {}

  protected static function funcReturn(mixed $c): CT4 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT4>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT4>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT4 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT4> {
    return vec[0.0,3.14,false,null,true];
  }
}
case type CT5 = float|float;

  
class CheckCT5<T as CT5> extends BaseCheck {
  const type T = CT5;
  const string NAME = 'CT5';

  <<__LateInit>>
  private CT5 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT5 $c): void {}

  protected static function funcReturn(mixed $c): CT5 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT5>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT5>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT5 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT5> {
    return vec[0.0,3.14];
  }
}
case type CT6 = string|float;

  
class CheckCT6<T as CT6> extends BaseCheck {
  const type T = CT6;
  const string NAME = 'CT6';

  <<__LateInit>>
  private CT6 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT6 $c): void {}

  protected static function funcReturn(mixed $c): CT6 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT6>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT6>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT6 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT6> {
    return vec['','hello world',0.0,3.14];
  }
}
case type CT7 = vec<mixed>|float;

  
class CheckCT7<T as CT7> extends BaseCheck {
  const type T = CT7;
  const string NAME = 'CT7';

  <<__LateInit>>
  private CT7 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT7 $c): void {}

  protected static function funcReturn(mixed $c): CT7 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT7>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT7>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT7 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT7> {
    return vec[0.0,3.14,vec[]];
  }
}
case type CT8 = ?bool|string;

  
class CheckCT8<T as CT8> extends BaseCheck {
  const type T = CT8;
  const string NAME = 'CT8';

  <<__LateInit>>
  private CT8 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT8 $c): void {}

  protected static function funcReturn(mixed $c): CT8 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT8>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT8>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT8 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT8> {
    return vec['','hello world',false,null,true];
  }
}
case type CT9 = float|string;

  
class CheckCT9<T as CT9> extends BaseCheck {
  const type T = CT9;
  const string NAME = 'CT9';

  <<__LateInit>>
  private CT9 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT9 $c): void {}

  protected static function funcReturn(mixed $c): CT9 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT9>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT9>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT9 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT9> {
    return vec['','hello world',0.0,3.14];
  }
}
case type CT10 = string|string;

  
class CheckCT10<T as CT10> extends BaseCheck {
  const type T = CT10;
  const string NAME = 'CT10';

  <<__LateInit>>
  private CT10 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT10 $c): void {}

  protected static function funcReturn(mixed $c): CT10 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT10>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT10>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT10 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT10> {
    return vec['','hello world'];
  }
}
case type CT11 = vec<mixed>|string;

  
class CheckCT11<T as CT11> extends BaseCheck {
  const type T = CT11;
  const string NAME = 'CT11';

  <<__LateInit>>
  private CT11 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT11 $c): void {}

  protected static function funcReturn(mixed $c): CT11 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT11>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT11>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT11 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT11> {
    return vec['','hello world',vec[]];
  }
}
case type CT12 = ?bool|vec<mixed>;

  
class CheckCT12<T as CT12> extends BaseCheck {
  const type T = CT12;
  const string NAME = 'CT12';

  <<__LateInit>>
  private CT12 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT12 $c): void {}

  protected static function funcReturn(mixed $c): CT12 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT12>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT12>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT12 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT12> {
    return vec[false,null,true,vec[]];
  }
}
case type CT13 = float|vec<mixed>;

  
class CheckCT13<T as CT13> extends BaseCheck {
  const type T = CT13;
  const string NAME = 'CT13';

  <<__LateInit>>
  private CT13 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT13 $c): void {}

  protected static function funcReturn(mixed $c): CT13 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT13>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT13>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT13 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT13> {
    return vec[0.0,3.14,vec[]];
  }
}
case type CT14 = string|vec<mixed>;

  
class CheckCT14<T as CT14> extends BaseCheck {
  const type T = CT14;
  const string NAME = 'CT14';

  <<__LateInit>>
  private CT14 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT14 $c): void {}

  protected static function funcReturn(mixed $c): CT14 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT14>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT14>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT14 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT14> {
    return vec['','hello world',vec[]];
  }
}
case type CT15 = vec<mixed>|vec<mixed>;

  
class CheckCT15<T as CT15> extends BaseCheck {
  const type T = CT15;
  const string NAME = 'CT15';

  <<__LateInit>>
  private CT15 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(CT15 $c): void {}

  protected static function funcReturn(mixed $c): CT15 {
    return $c;
  }

  protected static function funcGenericParam<Tx as CT15>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as CT15>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(CT15 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<CT15> {
    return vec[vec[]];
  }
}

  <<__EntryPoint>>
  function main(): void {
    CheckCT0::run();
CheckCT1::run();
CheckCT2::run();
CheckCT3::run();
CheckCT4::run();
CheckCT5::run();
CheckCT6::run();
CheckCT7::run();
CheckCT8::run();
CheckCT9::run();
CheckCT10::run();
CheckCT11::run();
CheckCT12::run();
CheckCT13::run();
CheckCT14::run();
CheckCT15::run();
  }