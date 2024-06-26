<?hh
  /**
   * THIS FILE IS @generated; DO NOT EDIT IT
   * To regenerate this file, run
   *
   *   buck run //hphp/hack/test:gen_case_type_tests
   **/

  <<file: __EnableUnstableFeatures('case_types')>>

  
    class StringishObj {
      public function __toString(): string { return ''; }
    }
    

  enum MyEnum : string {
    A = 'A';
    B = 'B';
  }

  enum class EC : nonnull {
    float A = 3.141;
    (float, float) B = tuple(0.0, 0.0);
  }

  interface I {}

  class InstanceOfI implements I {}
  

  trait MyTrait {}

  class UsesMyTrait {
    use MyTrait;
  }
abstract final class AbsFinal {}
class :my-xhp implements XHPChild {}
class AClass {}
class ReifiedClass<reify T> {}
function my_func(): void {}

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

  case type CT0 = (function(): void)|(function(): void);
              type AliasCT0 = CT0;

  
class CheckAliasCT0<T as AliasCT0> extends BaseCheck {
  const type T = AliasCT0;
  const string NAME = 'AliasCT0';

  <<__LateInit>>
  private AliasCT0 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT0 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT0 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT0>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT0>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT0 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT0> {
    return vec[() ==> {},my_func<>,vec['my_func']];
  }
}
case type CT1 = (mixed, mixed)|(function(): void);
              type AliasCT1 = CT1;

  
class CheckAliasCT1<T as AliasCT1> extends BaseCheck {
  const type T = AliasCT1;
  const string NAME = 'AliasCT1';

  <<__LateInit>>
  private AliasCT1 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT1 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT1 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT1>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT1>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT1 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT1> {
    return vec[() ==> {},my_func<>,tuple(0, 0),tuple(1, 2, 3),vec['my_func']];
  }
}
case type CT2 = ?bool|(function(): void);
              type AliasCT2 = CT2;

  
class CheckAliasCT2<T as AliasCT2> extends BaseCheck {
  const type T = AliasCT2;
  const string NAME = 'AliasCT2';

  <<__LateInit>>
  private AliasCT2 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT2 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT2 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT2>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT2>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT2 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT2> {
    return vec[() ==> {},false,my_func<>,null,true,vec['my_func']];
  }
}
case type CT3 = AClass|(function(): void);
              type AliasCT3 = CT3;

  
class CheckAliasCT3<T as AliasCT3> extends BaseCheck {
  const type T = AliasCT3;
  const string NAME = 'AliasCT3';

  <<__LateInit>>
  private AliasCT3 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT3 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT3 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT3>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT3>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT3 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT3> {
    return vec[() ==> {},my_func<>,new AClass(),vec['my_func']];
  }
}
case type CT4 = AbsFinal|(function(): void);
              type AliasCT4 = CT4;

  
class CheckAliasCT4<T as AliasCT4> extends BaseCheck {
  const type T = AliasCT4;
  const string NAME = 'AliasCT4';

  <<__LateInit>>
  private AliasCT4 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT4 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT4 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT4>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT4>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT4 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT4> {
    return vec[() ==> {},my_func<>,vec['my_func']];
  }
}
case type CT5 = Awaitable<num>|(function(): void);
              type AliasCT5 = CT5;

  
class CheckAliasCT5<T as AliasCT5> extends BaseCheck {
  const type T = AliasCT5;
  const string NAME = 'AliasCT5';

  <<__LateInit>>
  private AliasCT5 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT5 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT5 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT5>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT5>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT5 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT5> {
    return vec[() ==> {},async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },my_func<>,vec['my_func']];
  }
}
case type CT6 = Container<mixed>|(function(): void);
              type AliasCT6 = CT6;

  
class CheckAliasCT6<T as AliasCT6> extends BaseCheck {
  const type T = AliasCT6;
  const string NAME = 'AliasCT6';

  <<__LateInit>>
  private AliasCT6 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT6 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT6 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT6>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT6>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT6 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT6> {
    return vec[() ==> {},keyset[],my_func<>,vec['my_func'],vec[]];
  }
}
case type CT7 = HH\AnyArray<arraykey, mixed>|(function(): void);
              type AliasCT7 = CT7;

  
class CheckAliasCT7<T as AliasCT7> extends BaseCheck {
  const type T = AliasCT7;
  const string NAME = 'AliasCT7';

  <<__LateInit>>
  private AliasCT7 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT7 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT7 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT7>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT7>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT7 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT7> {
    return vec[() ==> {},dict[],keyset[],my_func<>,vec['my_func'],vec[]];
  }
}
case type CT8 = HH\EnumClass\Label<EC, float>|(function(): void);
              type AliasCT8 = CT8;

  
class CheckAliasCT8<T as AliasCT8> extends BaseCheck {
  const type T = AliasCT8;
  const string NAME = 'AliasCT8';

  <<__LateInit>>
  private AliasCT8 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT8 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT8 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT8>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT8>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT8 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT8> {
    return vec[#A,() ==> {},EC#B,my_func<>,vec['my_func']];
  }
}
case type CT9 = HH\FunctionRef<(function(): void)>|(function(): void);
              type AliasCT9 = CT9;

  
class CheckAliasCT9<T as AliasCT9> extends BaseCheck {
  const type T = AliasCT9;
  const string NAME = 'AliasCT9';

  <<__LateInit>>
  private AliasCT9 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT9 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT9 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT9>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT9>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT9 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT9> {
    return vec[() ==> {},my_func<>,vec['my_func']];
  }
}
case type CT10 = HH\MemberOf<EC, float>|(function(): void);
              type AliasCT10 = CT10;

  
class CheckAliasCT10<T as AliasCT10> extends BaseCheck {
  const type T = AliasCT10;
  const string NAME = 'AliasCT10';

  <<__LateInit>>
  private AliasCT10 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT10 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT10 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT10>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT10>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT10 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT10> {
    return vec[() ==> {},EC::A,EC::B,my_func<>,vec['my_func']];
  }
}
case type CT11 = I|(function(): void);
              type AliasCT11 = CT11;

  
class CheckAliasCT11<T as AliasCT11> extends BaseCheck {
  const type T = AliasCT11;
  const string NAME = 'AliasCT11';

  <<__LateInit>>
  private AliasCT11 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT11 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT11 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT11>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT11>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT11 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT11> {
    return vec[() ==> {},my_func<>,new InstanceOfI(),vec['my_func']];
  }
}
case type CT12 = KeyedContainer<arraykey, mixed>|(function(): void);
              type AliasCT12 = CT12;

  
class CheckAliasCT12<T as AliasCT12> extends BaseCheck {
  const type T = AliasCT12;
  const string NAME = 'AliasCT12';

  <<__LateInit>>
  private AliasCT12 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT12 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT12 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT12>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT12>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT12 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT12> {
    return vec[() ==> {},dict[],my_func<>,vec['my_func'],vec[]];
  }
}
case type CT13 = KeyedTraversable<arraykey, mixed>|(function(): void);
              type AliasCT13 = CT13;

  
class CheckAliasCT13<T as AliasCT13> extends BaseCheck {
  const type T = AliasCT13;
  const string NAME = 'AliasCT13';

  <<__LateInit>>
  private AliasCT13 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT13 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT13 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT13>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT13>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT13 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT13> {
    return vec[() ==> {},dict[],keyset[],my_func<>,vec['my_func'],vec[]];
  }
}
case type CT14 = MyEnum|(function(): void);
              type AliasCT14 = CT14;

  
class CheckAliasCT14<T as AliasCT14> extends BaseCheck {
  const type T = AliasCT14;
  const string NAME = 'AliasCT14';

  <<__LateInit>>
  private AliasCT14 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT14 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT14 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT14>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT14>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT14 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT14> {
    return vec['B',() ==> {},MyEnum::A,my_func<>,vec['my_func']];
  }
}
case type CT15 = MyTrait|(function(): void);
              type AliasCT15 = CT15;

  
class CheckAliasCT15<T as AliasCT15> extends BaseCheck {
  const type T = AliasCT15;
  const string NAME = 'AliasCT15';

  <<__LateInit>>
  private AliasCT15 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT15 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT15 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT15>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT15>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT15 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT15> {
    return vec[() ==> {},my_func<>,vec['my_func']];
  }
}
case type CT16 = ReifiedClass<null>|(function(): void);
              type AliasCT16 = CT16;

  
class CheckAliasCT16<T as AliasCT16> extends BaseCheck {
  const type T = AliasCT16;
  const string NAME = 'AliasCT16';

  <<__LateInit>>
  private AliasCT16 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT16 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT16 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT16>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT16>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT16 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT16> {
    return vec[() ==> {},my_func<>,new ReifiedClass<null>(),vec['my_func']];
  }
}
case type CT17 = Stringish|(function(): void);
              type AliasCT17 = CT17;

  
class CheckAliasCT17<T as AliasCT17> extends BaseCheck {
  const type T = AliasCT17;
  const string NAME = 'AliasCT17';

  <<__LateInit>>
  private AliasCT17 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT17 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT17 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT17>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT17>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT17 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT17> {
    return vec['','hello world',() ==> {},my_func<>,new StringishObj(),vec['my_func']];
  }
}
case type CT18 = Traversable<mixed>|(function(): void);
              type AliasCT18 = CT18;

  
class CheckAliasCT18<T as AliasCT18> extends BaseCheck {
  const type T = AliasCT18;
  const string NAME = 'AliasCT18';

  <<__LateInit>>
  private AliasCT18 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT18 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT18 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT18>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT18>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT18 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT18> {
    return vec[() ==> {},dict[],keyset[],my_func<>,vec['my_func'],vec[]];
  }
}
case type CT19 = XHPChild|(function(): void);
              type AliasCT19 = CT19;

  
class CheckAliasCT19<T as AliasCT19> extends BaseCheck {
  const type T = AliasCT19;
  const string NAME = 'AliasCT19';

  <<__LateInit>>
  private AliasCT19 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT19 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT19 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT19>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT19>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT19 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT19> {
    return vec['','hello world',() ==> {},0,1,<my-xhp/>,my_func<>,vec['my_func']];
  }
}
case type CT20 = arraykey|(function(): void);
              type AliasCT20 = CT20;

  
class CheckAliasCT20<T as AliasCT20> extends BaseCheck {
  const type T = AliasCT20;
  const string NAME = 'AliasCT20';

  <<__LateInit>>
  private AliasCT20 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT20 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT20 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT20>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT20>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT20 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT20> {
    return vec['','hello world',() ==> {},0,1,my_func<>,vec['my_func']];
  }
}
case type CT21 = bool|(function(): void);
              type AliasCT21 = CT21;

  
class CheckAliasCT21<T as AliasCT21> extends BaseCheck {
  const type T = AliasCT21;
  const string NAME = 'AliasCT21';

  <<__LateInit>>
  private AliasCT21 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT21 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT21 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT21>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT21>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT21 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT21> {
    return vec[() ==> {},false,my_func<>,true,vec['my_func']];
  }
}
case type CT22 = dict<arraykey, mixed>|(function(): void);
              type AliasCT22 = CT22;

  
class CheckAliasCT22<T as AliasCT22> extends BaseCheck {
  const type T = AliasCT22;
  const string NAME = 'AliasCT22';

  <<__LateInit>>
  private AliasCT22 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT22 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT22 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT22>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT22>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT22 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT22> {
    return vec[() ==> {},dict[],my_func<>,vec['my_func']];
  }
}
case type CT23 = dynamic|(function(): void);
              type AliasCT23 = CT23;

  
class CheckAliasCT23<T as AliasCT23> extends BaseCheck {
  const type T = AliasCT23;
  const string NAME = 'AliasCT23';

  <<__LateInit>>
  private AliasCT23 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT23 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT23 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT23>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT23>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT23 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT23> {
    return vec[() ==> {},false,my_func<>,null,shape('x' => 10),shape(),true,vec['my_func']];
  }
}
case type CT24 = float|(function(): void);
              type AliasCT24 = CT24;

  
class CheckAliasCT24<T as AliasCT24> extends BaseCheck {
  const type T = AliasCT24;
  const string NAME = 'AliasCT24';

  <<__LateInit>>
  private AliasCT24 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT24 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT24 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT24>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT24>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT24 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT24> {
    return vec[() ==> {},0.0,3.14,my_func<>,vec['my_func']];
  }
}
case type CT25 = int|(function(): void);
              type AliasCT25 = CT25;

  
class CheckAliasCT25<T as AliasCT25> extends BaseCheck {
  const type T = AliasCT25;
  const string NAME = 'AliasCT25';

  <<__LateInit>>
  private AliasCT25 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT25 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT25 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT25>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT25>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT25 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT25> {
    return vec[() ==> {},0,1,my_func<>,vec['my_func']];
  }
}
case type CT26 = keyset<arraykey>|(function(): void);
              type AliasCT26 = CT26;

  
class CheckAliasCT26<T as AliasCT26> extends BaseCheck {
  const type T = AliasCT26;
  const string NAME = 'AliasCT26';

  <<__LateInit>>
  private AliasCT26 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT26 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT26 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT26>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT26>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT26 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT26> {
    return vec[() ==> {},keyset[],my_func<>,vec['my_func']];
  }
}
case type CT27 = mixed|(function(): void);
              type AliasCT27 = CT27;

  
class CheckAliasCT27<T as AliasCT27> extends BaseCheck {
  const type T = AliasCT27;
  const string NAME = 'AliasCT27';

  <<__LateInit>>
  private AliasCT27 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT27 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT27 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT27>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT27>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT27 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT27> {
    return vec['','hello world',() ==> {},0,1,false,my_func<>,null,true,vec['my_func']];
  }
}
case type CT28 = nonnull|(function(): void);
              type AliasCT28 = CT28;

  
class CheckAliasCT28<T as AliasCT28> extends BaseCheck {
  const type T = AliasCT28;
  const string NAME = 'AliasCT28';

  <<__LateInit>>
  private AliasCT28 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT28 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT28 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT28>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT28>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT28 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT28> {
    return vec['','hello world',() ==> {},0,1,false,my_func<>,true,vec['my_func']];
  }
}
case type CT29 = noreturn|(function(): void);
              type AliasCT29 = CT29;

  
class CheckAliasCT29<T as AliasCT29> extends BaseCheck {
  const type T = AliasCT29;
  const string NAME = 'AliasCT29';

  <<__LateInit>>
  private AliasCT29 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT29 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT29 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT29>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT29>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT29 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT29> {
    return vec[() ==> {},my_func<>,vec['my_func']];
  }
}
case type CT30 = nothing|(function(): void);
              type AliasCT30 = CT30;

  
class CheckAliasCT30<T as AliasCT30> extends BaseCheck {
  const type T = AliasCT30;
  const string NAME = 'AliasCT30';

  <<__LateInit>>
  private AliasCT30 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT30 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT30 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT30>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT30>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT30 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT30> {
    return vec[() ==> {},my_func<>,vec['my_func']];
  }
}
case type CT31 = null|(function(): void);
              type AliasCT31 = CT31;

  
class CheckAliasCT31<T as AliasCT31> extends BaseCheck {
  const type T = AliasCT31;
  const string NAME = 'AliasCT31';

  <<__LateInit>>
  private AliasCT31 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT31 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT31 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT31>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT31>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT31 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT31> {
    return vec[() ==> {},my_func<>,null,vec['my_func']];
  }
}
case type CT32 = num|(function(): void);
              type AliasCT32 = CT32;

  
class CheckAliasCT32<T as AliasCT32> extends BaseCheck {
  const type T = AliasCT32;
  const string NAME = 'AliasCT32';

  <<__LateInit>>
  private AliasCT32 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT32 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT32 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT32>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT32>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT32 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT32> {
    return vec[() ==> {},0,0.0,1,3.14,my_func<>,vec['my_func']];
  }
}
case type CT33 = resource|(function(): void);
              type AliasCT33 = CT33;

  
class CheckAliasCT33<T as AliasCT33> extends BaseCheck {
  const type T = AliasCT33;
  const string NAME = 'AliasCT33';

  <<__LateInit>>
  private AliasCT33 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT33 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT33 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT33>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT33>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT33 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT33> {
    return vec[() ==> {},imagecreate(10, 10),my_func<>,vec['my_func']];
  }
}
case type CT34 = shape(...)|(function(): void);
              type AliasCT34 = CT34;

  
class CheckAliasCT34<T as AliasCT34> extends BaseCheck {
  const type T = AliasCT34;
  const string NAME = 'AliasCT34';

  <<__LateInit>>
  private AliasCT34 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT34 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT34 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT34>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT34>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT34 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT34> {
    return vec[() ==> {},my_func<>,shape('x' => 10),shape(),vec['my_func']];
  }
}
case type CT35 = string|(function(): void);
              type AliasCT35 = CT35;

  
class CheckAliasCT35<T as AliasCT35> extends BaseCheck {
  const type T = AliasCT35;
  const string NAME = 'AliasCT35';

  <<__LateInit>>
  private AliasCT35 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT35 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT35 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT35>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT35>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT35 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT35> {
    return vec['','hello world',() ==> {},my_func<>,vec['my_func']];
  }
}
case type CT36 = vec<mixed>|(function(): void);
              type AliasCT36 = CT36;

  
class CheckAliasCT36<T as AliasCT36> extends BaseCheck {
  const type T = AliasCT36;
  const string NAME = 'AliasCT36';

  <<__LateInit>>
  private AliasCT36 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT36 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT36 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT36>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT36>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT36 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT36> {
    return vec[() ==> {},my_func<>,vec['my_func'],vec[]];
  }
}
case type CT37 = vec_or_dict<string>|(function(): void);
              type AliasCT37 = CT37;

  
class CheckAliasCT37<T as AliasCT37> extends BaseCheck {
  const type T = AliasCT37;
  const string NAME = 'AliasCT37';

  <<__LateInit>>
  private AliasCT37 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT37 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT37 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT37>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT37>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT37 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT37> {
    return vec[() ==> {},dict[],my_func<>,vec['my_func'],vec[]];
  }
}
case type CT38 = void|(function(): void);
              type AliasCT38 = CT38;

  
class CheckAliasCT38<T as AliasCT38> extends BaseCheck {
  const type T = AliasCT38;
  const string NAME = 'AliasCT38';

  <<__LateInit>>
  private AliasCT38 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT38 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT38 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT38>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT38>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT38 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT38> {
    return vec[() ==> {},my_func<>,null,vec['my_func']];
  }
}
case type CT39 = (mixed, mixed)|(mixed, mixed);
              type AliasCT39 = CT39;

  
class CheckAliasCT39<T as AliasCT39> extends BaseCheck {
  const type T = AliasCT39;
  const string NAME = 'AliasCT39';

  <<__LateInit>>
  private AliasCT39 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT39 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT39 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT39>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT39>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT39 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT39> {
    return vec[tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT40 = ?bool|(mixed, mixed);
              type AliasCT40 = CT40;

  
class CheckAliasCT40<T as AliasCT40> extends BaseCheck {
  const type T = AliasCT40;
  const string NAME = 'AliasCT40';

  <<__LateInit>>
  private AliasCT40 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT40 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT40 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT40>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT40>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT40 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT40> {
    return vec[false,null,true,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT41 = AClass|(mixed, mixed);
              type AliasCT41 = CT41;

  
class CheckAliasCT41<T as AliasCT41> extends BaseCheck {
  const type T = AliasCT41;
  const string NAME = 'AliasCT41';

  <<__LateInit>>
  private AliasCT41 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT41 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT41 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT41>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT41>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT41 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT41> {
    return vec[new AClass(),tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT42 = AbsFinal|(mixed, mixed);
              type AliasCT42 = CT42;

  
class CheckAliasCT42<T as AliasCT42> extends BaseCheck {
  const type T = AliasCT42;
  const string NAME = 'AliasCT42';

  <<__LateInit>>
  private AliasCT42 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT42 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT42 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT42>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT42>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT42 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT42> {
    return vec[tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT43 = Awaitable<num>|(mixed, mixed);
              type AliasCT43 = CT43;

  
class CheckAliasCT43<T as AliasCT43> extends BaseCheck {
  const type T = AliasCT43;
  const string NAME = 'AliasCT43';

  <<__LateInit>>
  private AliasCT43 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT43 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT43 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT43>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT43>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT43 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT43> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT44 = Container<mixed>|(mixed, mixed);
              type AliasCT44 = CT44;

  
class CheckAliasCT44<T as AliasCT44> extends BaseCheck {
  const type T = AliasCT44;
  const string NAME = 'AliasCT44';

  <<__LateInit>>
  private AliasCT44 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT44 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT44 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT44>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT44>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT44 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT44> {
    return vec[keyset[],tuple(0, 0),tuple(1, 2, 3),vec[]];
  }
}
case type CT45 = HH\AnyArray<arraykey, mixed>|(mixed, mixed);
              type AliasCT45 = CT45;

  
class CheckAliasCT45<T as AliasCT45> extends BaseCheck {
  const type T = AliasCT45;
  const string NAME = 'AliasCT45';

  <<__LateInit>>
  private AliasCT45 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT45 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT45 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT45>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT45>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT45 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT45> {
    return vec[dict[],keyset[],tuple(0, 0),tuple(1, 2, 3),vec[]];
  }
}
case type CT46 = HH\EnumClass\Label<EC, float>|(mixed, mixed);
              type AliasCT46 = CT46;

  
class CheckAliasCT46<T as AliasCT46> extends BaseCheck {
  const type T = AliasCT46;
  const string NAME = 'AliasCT46';

  <<__LateInit>>
  private AliasCT46 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT46 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT46 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT46>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT46>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT46 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT46> {
    return vec[#A,EC#B,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT47 = HH\FunctionRef<(function(): void)>|(mixed, mixed);
              type AliasCT47 = CT47;

  
class CheckAliasCT47<T as AliasCT47> extends BaseCheck {
  const type T = AliasCT47;
  const string NAME = 'AliasCT47';

  <<__LateInit>>
  private AliasCT47 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT47 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT47 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT47>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT47>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT47 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT47> {
    return vec[my_func<>,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT48 = HH\MemberOf<EC, float>|(mixed, mixed);
              type AliasCT48 = CT48;

  
class CheckAliasCT48<T as AliasCT48> extends BaseCheck {
  const type T = AliasCT48;
  const string NAME = 'AliasCT48';

  <<__LateInit>>
  private AliasCT48 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT48 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT48 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT48>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT48>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT48 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT48> {
    return vec[EC::A,EC::B,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT49 = I|(mixed, mixed);
              type AliasCT49 = CT49;

  
class CheckAliasCT49<T as AliasCT49> extends BaseCheck {
  const type T = AliasCT49;
  const string NAME = 'AliasCT49';

  <<__LateInit>>
  private AliasCT49 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT49 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT49 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT49>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT49>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT49 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT49> {
    return vec[new InstanceOfI(),tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT50 = KeyedContainer<arraykey, mixed>|(mixed, mixed);
              type AliasCT50 = CT50;

  
class CheckAliasCT50<T as AliasCT50> extends BaseCheck {
  const type T = AliasCT50;
  const string NAME = 'AliasCT50';

  <<__LateInit>>
  private AliasCT50 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT50 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT50 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT50>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT50>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT50 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT50> {
    return vec[dict[],tuple(0, 0),tuple(1, 2, 3),vec[]];
  }
}
case type CT51 = KeyedTraversable<arraykey, mixed>|(mixed, mixed);
              type AliasCT51 = CT51;

  
class CheckAliasCT51<T as AliasCT51> extends BaseCheck {
  const type T = AliasCT51;
  const string NAME = 'AliasCT51';

  <<__LateInit>>
  private AliasCT51 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT51 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT51 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT51>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT51>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT51 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT51> {
    return vec[dict[],keyset[],tuple(0, 0),tuple(1, 2, 3),vec[]];
  }
}
case type CT52 = MyTrait|(mixed, mixed);
              type AliasCT52 = CT52;

  
class CheckAliasCT52<T as AliasCT52> extends BaseCheck {
  const type T = AliasCT52;
  const string NAME = 'AliasCT52';

  <<__LateInit>>
  private AliasCT52 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT52 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT52 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT52>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT52>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT52 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT52> {
    return vec[tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT53 = ReifiedClass<null>|(mixed, mixed);
              type AliasCT53 = CT53;

  
class CheckAliasCT53<T as AliasCT53> extends BaseCheck {
  const type T = AliasCT53;
  const string NAME = 'AliasCT53';

  <<__LateInit>>
  private AliasCT53 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT53 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT53 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT53>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT53>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT53 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT53> {
    return vec[new ReifiedClass<null>(),tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT54 = Stringish|(mixed, mixed);
              type AliasCT54 = CT54;

  
class CheckAliasCT54<T as AliasCT54> extends BaseCheck {
  const type T = AliasCT54;
  const string NAME = 'AliasCT54';

  <<__LateInit>>
  private AliasCT54 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT54 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT54 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT54>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT54>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT54 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT54> {
    return vec['','hello world',new StringishObj(),tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT55 = Traversable<mixed>|(mixed, mixed);
              type AliasCT55 = CT55;

  
class CheckAliasCT55<T as AliasCT55> extends BaseCheck {
  const type T = AliasCT55;
  const string NAME = 'AliasCT55';

  <<__LateInit>>
  private AliasCT55 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT55 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT55 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT55>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT55>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT55 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT55> {
    return vec[dict[],keyset[],tuple(0, 0),tuple(1, 2, 3),vec[]];
  }
}
case type CT56 = XHPChild|(mixed, mixed);
              type AliasCT56 = CT56;

  
class CheckAliasCT56<T as AliasCT56> extends BaseCheck {
  const type T = AliasCT56;
  const string NAME = 'AliasCT56';

  <<__LateInit>>
  private AliasCT56 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT56 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT56 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT56>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT56>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT56 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT56> {
    return vec['','hello world',0,1,<my-xhp/>,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT57 = arraykey|(mixed, mixed);
              type AliasCT57 = CT57;

  
class CheckAliasCT57<T as AliasCT57> extends BaseCheck {
  const type T = AliasCT57;
  const string NAME = 'AliasCT57';

  <<__LateInit>>
  private AliasCT57 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT57 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT57 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT57>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT57>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT57 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT57> {
    return vec['','hello world',0,1,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT58 = bool|(mixed, mixed);
              type AliasCT58 = CT58;

  
class CheckAliasCT58<T as AliasCT58> extends BaseCheck {
  const type T = AliasCT58;
  const string NAME = 'AliasCT58';

  <<__LateInit>>
  private AliasCT58 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT58 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT58 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT58>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT58>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT58 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT58> {
    return vec[false,true,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT59 = dict<arraykey, mixed>|(mixed, mixed);
              type AliasCT59 = CT59;

  
class CheckAliasCT59<T as AliasCT59> extends BaseCheck {
  const type T = AliasCT59;
  const string NAME = 'AliasCT59';

  <<__LateInit>>
  private AliasCT59 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT59 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT59 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT59>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT59>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT59 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT59> {
    return vec[dict[],tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT60 = dynamic|(mixed, mixed);
              type AliasCT60 = CT60;

  
class CheckAliasCT60<T as AliasCT60> extends BaseCheck {
  const type T = AliasCT60;
  const string NAME = 'AliasCT60';

  <<__LateInit>>
  private AliasCT60 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT60 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT60 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT60>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT60>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT60 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT60> {
    return vec[false,null,shape('x' => 10),shape(),true,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT61 = float|(mixed, mixed);
              type AliasCT61 = CT61;

  
class CheckAliasCT61<T as AliasCT61> extends BaseCheck {
  const type T = AliasCT61;
  const string NAME = 'AliasCT61';

  <<__LateInit>>
  private AliasCT61 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT61 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT61 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT61>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT61>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT61 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT61> {
    return vec[0.0,3.14,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT62 = int|(mixed, mixed);
              type AliasCT62 = CT62;

  
class CheckAliasCT62<T as AliasCT62> extends BaseCheck {
  const type T = AliasCT62;
  const string NAME = 'AliasCT62';

  <<__LateInit>>
  private AliasCT62 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT62 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT62 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT62>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT62>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT62 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT62> {
    return vec[0,1,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT63 = keyset<arraykey>|(mixed, mixed);
              type AliasCT63 = CT63;

  
class CheckAliasCT63<T as AliasCT63> extends BaseCheck {
  const type T = AliasCT63;
  const string NAME = 'AliasCT63';

  <<__LateInit>>
  private AliasCT63 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT63 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT63 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT63>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT63>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT63 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT63> {
    return vec[keyset[],tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT64 = mixed|(mixed, mixed);
              type AliasCT64 = CT64;

  
class CheckAliasCT64<T as AliasCT64> extends BaseCheck {
  const type T = AliasCT64;
  const string NAME = 'AliasCT64';

  <<__LateInit>>
  private AliasCT64 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT64 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT64 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT64>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT64>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT64 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT64> {
    return vec['','hello world',0,1,false,null,true,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT65 = nonnull|(mixed, mixed);
              type AliasCT65 = CT65;

  
class CheckAliasCT65<T as AliasCT65> extends BaseCheck {
  const type T = AliasCT65;
  const string NAME = 'AliasCT65';

  <<__LateInit>>
  private AliasCT65 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT65 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT65 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT65>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT65>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT65 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT65> {
    return vec['','hello world',0,1,false,true,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT66 = noreturn|(mixed, mixed);
              type AliasCT66 = CT66;

  
class CheckAliasCT66<T as AliasCT66> extends BaseCheck {
  const type T = AliasCT66;
  const string NAME = 'AliasCT66';

  <<__LateInit>>
  private AliasCT66 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT66 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT66 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT66>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT66>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT66 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT66> {
    return vec[tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT67 = nothing|(mixed, mixed);
              type AliasCT67 = CT67;

  
class CheckAliasCT67<T as AliasCT67> extends BaseCheck {
  const type T = AliasCT67;
  const string NAME = 'AliasCT67';

  <<__LateInit>>
  private AliasCT67 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT67 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT67 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT67>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT67>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT67 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT67> {
    return vec[tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT68 = null|(mixed, mixed);
              type AliasCT68 = CT68;

  
class CheckAliasCT68<T as AliasCT68> extends BaseCheck {
  const type T = AliasCT68;
  const string NAME = 'AliasCT68';

  <<__LateInit>>
  private AliasCT68 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT68 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT68 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT68>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT68>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT68 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT68> {
    return vec[null,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT69 = num|(mixed, mixed);
              type AliasCT69 = CT69;

  
class CheckAliasCT69<T as AliasCT69> extends BaseCheck {
  const type T = AliasCT69;
  const string NAME = 'AliasCT69';

  <<__LateInit>>
  private AliasCT69 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT69 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT69 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT69>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT69>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT69 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT69> {
    return vec[0,0.0,1,3.14,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT70 = resource|(mixed, mixed);
              type AliasCT70 = CT70;

  
class CheckAliasCT70<T as AliasCT70> extends BaseCheck {
  const type T = AliasCT70;
  const string NAME = 'AliasCT70';

  <<__LateInit>>
  private AliasCT70 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT70 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT70 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT70>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT70>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT70 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT70> {
    return vec[imagecreate(10, 10),tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT71 = shape(...)|(mixed, mixed);
              type AliasCT71 = CT71;

  
class CheckAliasCT71<T as AliasCT71> extends BaseCheck {
  const type T = AliasCT71;
  const string NAME = 'AliasCT71';

  <<__LateInit>>
  private AliasCT71 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT71 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT71 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT71>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT71>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT71 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT71> {
    return vec[shape('x' => 10),shape(),tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT72 = string|(mixed, mixed);
              type AliasCT72 = CT72;

  
class CheckAliasCT72<T as AliasCT72> extends BaseCheck {
  const type T = AliasCT72;
  const string NAME = 'AliasCT72';

  <<__LateInit>>
  private AliasCT72 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT72 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT72 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT72>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT72>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT72 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT72> {
    return vec['','hello world',tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT73 = vec<mixed>|(mixed, mixed);
              type AliasCT73 = CT73;

  
class CheckAliasCT73<T as AliasCT73> extends BaseCheck {
  const type T = AliasCT73;
  const string NAME = 'AliasCT73';

  <<__LateInit>>
  private AliasCT73 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT73 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT73 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT73>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT73>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT73 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT73> {
    return vec[tuple(0, 0),tuple(1, 2, 3),vec[]];
  }
}
case type CT74 = vec_or_dict<string>|(mixed, mixed);
              type AliasCT74 = CT74;

  
class CheckAliasCT74<T as AliasCT74> extends BaseCheck {
  const type T = AliasCT74;
  const string NAME = 'AliasCT74';

  <<__LateInit>>
  private AliasCT74 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT74 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT74 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT74>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT74>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT74 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT74> {
    return vec[dict[],tuple(0, 0),tuple(1, 2, 3),vec[]];
  }
}
case type CT75 = void|(mixed, mixed);
              type AliasCT75 = CT75;

  
class CheckAliasCT75<T as AliasCT75> extends BaseCheck {
  const type T = AliasCT75;
  const string NAME = 'AliasCT75';

  <<__LateInit>>
  private AliasCT75 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT75 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT75 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT75>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT75>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT75 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT75> {
    return vec[null,tuple(0, 0),tuple(1, 2, 3)];
  }
}
case type CT76 = ?bool|?bool;
              type AliasCT76 = CT76;

  
class CheckAliasCT76<T as AliasCT76> extends BaseCheck {
  const type T = AliasCT76;
  const string NAME = 'AliasCT76';

  <<__LateInit>>
  private AliasCT76 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT76 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT76 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT76>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT76>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT76 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT76> {
    return vec[false,null,true];
  }
}
case type CT77 = AClass|?bool;
              type AliasCT77 = CT77;

  
class CheckAliasCT77<T as AliasCT77> extends BaseCheck {
  const type T = AliasCT77;
  const string NAME = 'AliasCT77';

  <<__LateInit>>
  private AliasCT77 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT77 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT77 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT77>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT77>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT77 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT77> {
    return vec[false,new AClass(),null,true];
  }
}
case type CT78 = AbsFinal|?bool;
              type AliasCT78 = CT78;

  
class CheckAliasCT78<T as AliasCT78> extends BaseCheck {
  const type T = AliasCT78;
  const string NAME = 'AliasCT78';

  <<__LateInit>>
  private AliasCT78 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT78 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT78 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT78>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT78>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT78 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT78> {
    return vec[false,null,true];
  }
}
case type CT79 = Awaitable<num>|?bool;
              type AliasCT79 = CT79;

  
class CheckAliasCT79<T as AliasCT79> extends BaseCheck {
  const type T = AliasCT79;
  const string NAME = 'AliasCT79';

  <<__LateInit>>
  private AliasCT79 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT79 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT79 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT79>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT79>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT79 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT79> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },false,null,true];
  }
}
case type CT80 = Container<mixed>|?bool;
              type AliasCT80 = CT80;

  
class CheckAliasCT80<T as AliasCT80> extends BaseCheck {
  const type T = AliasCT80;
  const string NAME = 'AliasCT80';

  <<__LateInit>>
  private AliasCT80 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT80 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT80 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT80>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT80>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT80 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT80> {
    return vec[false,keyset[],null,true,vec[]];
  }
}
case type CT81 = HH\AnyArray<arraykey, mixed>|?bool;
              type AliasCT81 = CT81;

  
class CheckAliasCT81<T as AliasCT81> extends BaseCheck {
  const type T = AliasCT81;
  const string NAME = 'AliasCT81';

  <<__LateInit>>
  private AliasCT81 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT81 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT81 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT81>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT81>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT81 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT81> {
    return vec[dict[],false,keyset[],null,true,vec[]];
  }
}
case type CT82 = HH\EnumClass\Label<EC, float>|?bool;
              type AliasCT82 = CT82;

  
class CheckAliasCT82<T as AliasCT82> extends BaseCheck {
  const type T = AliasCT82;
  const string NAME = 'AliasCT82';

  <<__LateInit>>
  private AliasCT82 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT82 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT82 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT82>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT82>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT82 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT82> {
    return vec[#A,EC#B,false,null,true];
  }
}
case type CT83 = HH\FunctionRef<(function(): void)>|?bool;
              type AliasCT83 = CT83;

  
class CheckAliasCT83<T as AliasCT83> extends BaseCheck {
  const type T = AliasCT83;
  const string NAME = 'AliasCT83';

  <<__LateInit>>
  private AliasCT83 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT83 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT83 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT83>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT83>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT83 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT83> {
    return vec[false,my_func<>,null,true];
  }
}
case type CT84 = HH\MemberOf<EC, float>|?bool;
              type AliasCT84 = CT84;

  
class CheckAliasCT84<T as AliasCT84> extends BaseCheck {
  const type T = AliasCT84;
  const string NAME = 'AliasCT84';

  <<__LateInit>>
  private AliasCT84 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT84 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT84 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT84>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT84>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT84 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT84> {
    return vec[EC::A,EC::B,false,null,true];
  }
}
case type CT85 = I|?bool;
              type AliasCT85 = CT85;

  
class CheckAliasCT85<T as AliasCT85> extends BaseCheck {
  const type T = AliasCT85;
  const string NAME = 'AliasCT85';

  <<__LateInit>>
  private AliasCT85 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT85 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT85 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT85>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT85>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT85 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT85> {
    return vec[false,new InstanceOfI(),null,true];
  }
}
case type CT86 = KeyedContainer<arraykey, mixed>|?bool;
              type AliasCT86 = CT86;

  
class CheckAliasCT86<T as AliasCT86> extends BaseCheck {
  const type T = AliasCT86;
  const string NAME = 'AliasCT86';

  <<__LateInit>>
  private AliasCT86 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT86 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT86 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT86>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT86>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT86 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT86> {
    return vec[dict[],false,null,true,vec[]];
  }
}
case type CT87 = KeyedTraversable<arraykey, mixed>|?bool;
              type AliasCT87 = CT87;

  
class CheckAliasCT87<T as AliasCT87> extends BaseCheck {
  const type T = AliasCT87;
  const string NAME = 'AliasCT87';

  <<__LateInit>>
  private AliasCT87 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT87 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT87 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT87>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT87>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT87 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT87> {
    return vec[dict[],false,keyset[],null,true,vec[]];
  }
}
case type CT88 = MyTrait|?bool;
              type AliasCT88 = CT88;

  
class CheckAliasCT88<T as AliasCT88> extends BaseCheck {
  const type T = AliasCT88;
  const string NAME = 'AliasCT88';

  <<__LateInit>>
  private AliasCT88 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT88 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT88 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT88>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT88>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT88 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT88> {
    return vec[false,null,true];
  }
}
case type CT89 = ReifiedClass<null>|?bool;
              type AliasCT89 = CT89;

  
class CheckAliasCT89<T as AliasCT89> extends BaseCheck {
  const type T = AliasCT89;
  const string NAME = 'AliasCT89';

  <<__LateInit>>
  private AliasCT89 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT89 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT89 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT89>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT89>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT89 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT89> {
    return vec[false,new ReifiedClass<null>(),null,true];
  }
}
case type CT90 = Stringish|?bool;
              type AliasCT90 = CT90;

  
class CheckAliasCT90<T as AliasCT90> extends BaseCheck {
  const type T = AliasCT90;
  const string NAME = 'AliasCT90';

  <<__LateInit>>
  private AliasCT90 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT90 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT90 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT90>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT90>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT90 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT90> {
    return vec['','hello world',false,new StringishObj(),null,true];
  }
}
case type CT91 = Traversable<mixed>|?bool;
              type AliasCT91 = CT91;

  
class CheckAliasCT91<T as AliasCT91> extends BaseCheck {
  const type T = AliasCT91;
  const string NAME = 'AliasCT91';

  <<__LateInit>>
  private AliasCT91 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT91 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT91 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT91>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT91>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT91 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT91> {
    return vec[dict[],false,keyset[],null,true,vec[]];
  }
}
case type CT92 = XHPChild|?bool;
              type AliasCT92 = CT92;

  
class CheckAliasCT92<T as AliasCT92> extends BaseCheck {
  const type T = AliasCT92;
  const string NAME = 'AliasCT92';

  <<__LateInit>>
  private AliasCT92 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT92 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT92 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT92>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT92>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT92 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT92> {
    return vec['','hello world',0,1,<my-xhp/>,false,null,true];
  }
}
case type CT93 = arraykey|?bool;
              type AliasCT93 = CT93;

  
class CheckAliasCT93<T as AliasCT93> extends BaseCheck {
  const type T = AliasCT93;
  const string NAME = 'AliasCT93';

  <<__LateInit>>
  private AliasCT93 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT93 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT93 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT93>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT93>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT93 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT93> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT94 = bool|?bool;
              type AliasCT94 = CT94;

  
class CheckAliasCT94<T as AliasCT94> extends BaseCheck {
  const type T = AliasCT94;
  const string NAME = 'AliasCT94';

  <<__LateInit>>
  private AliasCT94 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT94 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT94 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT94>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT94>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT94 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT94> {
    return vec[false,null,true];
  }
}
case type CT95 = dict<arraykey, mixed>|?bool;
              type AliasCT95 = CT95;

  
class CheckAliasCT95<T as AliasCT95> extends BaseCheck {
  const type T = AliasCT95;
  const string NAME = 'AliasCT95';

  <<__LateInit>>
  private AliasCT95 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT95 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT95 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT95>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT95>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT95 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT95> {
    return vec[dict[],false,null,true];
  }
}
case type CT96 = dynamic|?bool;
              type AliasCT96 = CT96;

  
class CheckAliasCT96<T as AliasCT96> extends BaseCheck {
  const type T = AliasCT96;
  const string NAME = 'AliasCT96';

  <<__LateInit>>
  private AliasCT96 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT96 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT96 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT96>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT96>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT96 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT96> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT97 = float|?bool;
              type AliasCT97 = CT97;

  
class CheckAliasCT97<T as AliasCT97> extends BaseCheck {
  const type T = AliasCT97;
  const string NAME = 'AliasCT97';

  <<__LateInit>>
  private AliasCT97 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT97 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT97 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT97>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT97>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT97 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT97> {
    return vec[0.0,3.14,false,null,true];
  }
}
case type CT98 = int|?bool;
              type AliasCT98 = CT98;

  
class CheckAliasCT98<T as AliasCT98> extends BaseCheck {
  const type T = AliasCT98;
  const string NAME = 'AliasCT98';

  <<__LateInit>>
  private AliasCT98 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT98 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT98 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT98>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT98>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT98 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT98> {
    return vec[0,1,false,null,true];
  }
}
case type CT99 = keyset<arraykey>|?bool;
              type AliasCT99 = CT99;

  
class CheckAliasCT99<T as AliasCT99> extends BaseCheck {
  const type T = AliasCT99;
  const string NAME = 'AliasCT99';

  <<__LateInit>>
  private AliasCT99 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT99 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT99 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT99>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT99>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT99 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT99> {
    return vec[false,keyset[],null,true];
  }
}
case type CT100 = mixed|?bool;
              type AliasCT100 = CT100;

  
class CheckAliasCT100<T as AliasCT100> extends BaseCheck {
  const type T = AliasCT100;
  const string NAME = 'AliasCT100';

  <<__LateInit>>
  private AliasCT100 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT100 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT100 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT100>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT100>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT100 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT100> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT101 = nonnull|?bool;
              type AliasCT101 = CT101;

  
class CheckAliasCT101<T as AliasCT101> extends BaseCheck {
  const type T = AliasCT101;
  const string NAME = 'AliasCT101';

  <<__LateInit>>
  private AliasCT101 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT101 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT101 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT101>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT101>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT101 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT101> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT102 = noreturn|?bool;
              type AliasCT102 = CT102;

  
class CheckAliasCT102<T as AliasCT102> extends BaseCheck {
  const type T = AliasCT102;
  const string NAME = 'AliasCT102';

  <<__LateInit>>
  private AliasCT102 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT102 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT102 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT102>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT102>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT102 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT102> {
    return vec[false,null,true];
  }
}
case type CT103 = nothing|?bool;
              type AliasCT103 = CT103;

  
class CheckAliasCT103<T as AliasCT103> extends BaseCheck {
  const type T = AliasCT103;
  const string NAME = 'AliasCT103';

  <<__LateInit>>
  private AliasCT103 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT103 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT103 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT103>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT103>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT103 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT103> {
    return vec[false,null,true];
  }
}
case type CT104 = null|?bool;
              type AliasCT104 = CT104;

  
class CheckAliasCT104<T as AliasCT104> extends BaseCheck {
  const type T = AliasCT104;
  const string NAME = 'AliasCT104';

  <<__LateInit>>
  private AliasCT104 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT104 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT104 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT104>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT104>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT104 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT104> {
    return vec[false,null,true];
  }
}
case type CT105 = num|?bool;
              type AliasCT105 = CT105;

  
class CheckAliasCT105<T as AliasCT105> extends BaseCheck {
  const type T = AliasCT105;
  const string NAME = 'AliasCT105';

  <<__LateInit>>
  private AliasCT105 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT105 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT105 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT105>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT105>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT105 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT105> {
    return vec[0,0.0,1,3.14,false,null,true];
  }
}
case type CT106 = resource|?bool;
              type AliasCT106 = CT106;

  
class CheckAliasCT106<T as AliasCT106> extends BaseCheck {
  const type T = AliasCT106;
  const string NAME = 'AliasCT106';

  <<__LateInit>>
  private AliasCT106 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT106 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT106 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT106>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT106>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT106 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT106> {
    return vec[false,imagecreate(10, 10),null,true];
  }
}
case type CT107 = shape(...)|?bool;
              type AliasCT107 = CT107;

  
class CheckAliasCT107<T as AliasCT107> extends BaseCheck {
  const type T = AliasCT107;
  const string NAME = 'AliasCT107';

  <<__LateInit>>
  private AliasCT107 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT107 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT107 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT107>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT107>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT107 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT107> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT108 = string|?bool;
              type AliasCT108 = CT108;

  
class CheckAliasCT108<T as AliasCT108> extends BaseCheck {
  const type T = AliasCT108;
  const string NAME = 'AliasCT108';

  <<__LateInit>>
  private AliasCT108 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT108 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT108 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT108>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT108>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT108 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT108> {
    return vec['','hello world',false,null,true];
  }
}
case type CT109 = vec<mixed>|?bool;
              type AliasCT109 = CT109;

  
class CheckAliasCT109<T as AliasCT109> extends BaseCheck {
  const type T = AliasCT109;
  const string NAME = 'AliasCT109';

  <<__LateInit>>
  private AliasCT109 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT109 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT109 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT109>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT109>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT109 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT109> {
    return vec[false,null,true,vec[]];
  }
}
case type CT110 = vec_or_dict<string>|?bool;
              type AliasCT110 = CT110;

  
class CheckAliasCT110<T as AliasCT110> extends BaseCheck {
  const type T = AliasCT110;
  const string NAME = 'AliasCT110';

  <<__LateInit>>
  private AliasCT110 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT110 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT110 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT110>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT110>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT110 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT110> {
    return vec[dict[],false,null,true,vec[]];
  }
}
case type CT111 = void|?bool;
              type AliasCT111 = CT111;

  
class CheckAliasCT111<T as AliasCT111> extends BaseCheck {
  const type T = AliasCT111;
  const string NAME = 'AliasCT111';

  <<__LateInit>>
  private AliasCT111 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT111 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT111 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT111>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT111>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT111 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT111> {
    return vec[false,null,true];
  }
}
case type CT112 = AClass|AClass;
              type AliasCT112 = CT112;

  
class CheckAliasCT112<T as AliasCT112> extends BaseCheck {
  const type T = AliasCT112;
  const string NAME = 'AliasCT112';

  <<__LateInit>>
  private AliasCT112 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT112 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT112 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT112>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT112>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT112 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT112> {
    return vec[new AClass()];
  }
}
case type CT113 = AbsFinal|AClass;
              type AliasCT113 = CT113;

  
class CheckAliasCT113<T as AliasCT113> extends BaseCheck {
  const type T = AliasCT113;
  const string NAME = 'AliasCT113';

  <<__LateInit>>
  private AliasCT113 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT113 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT113 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT113>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT113>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT113 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT113> {
    return vec[new AClass()];
  }
}
case type CT114 = Awaitable<num>|AClass;
              type AliasCT114 = CT114;

  
class CheckAliasCT114<T as AliasCT114> extends BaseCheck {
  const type T = AliasCT114;
  const string NAME = 'AliasCT114';

  <<__LateInit>>
  private AliasCT114 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT114 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT114 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT114>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT114>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT114 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT114> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },new AClass()];
  }
}
case type CT115 = Container<mixed>|AClass;
              type AliasCT115 = CT115;

  
class CheckAliasCT115<T as AliasCT115> extends BaseCheck {
  const type T = AliasCT115;
  const string NAME = 'AliasCT115';

  <<__LateInit>>
  private AliasCT115 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT115 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT115 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT115>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT115>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT115 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT115> {
    return vec[keyset[],new AClass(),vec[]];
  }
}
case type CT116 = HH\AnyArray<arraykey, mixed>|AClass;
              type AliasCT116 = CT116;

  
class CheckAliasCT116<T as AliasCT116> extends BaseCheck {
  const type T = AliasCT116;
  const string NAME = 'AliasCT116';

  <<__LateInit>>
  private AliasCT116 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT116 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT116 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT116>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT116>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT116 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT116> {
    return vec[dict[],keyset[],new AClass(),vec[]];
  }
}
case type CT117 = HH\EnumClass\Label<EC, float>|AClass;
              type AliasCT117 = CT117;

  
class CheckAliasCT117<T as AliasCT117> extends BaseCheck {
  const type T = AliasCT117;
  const string NAME = 'AliasCT117';

  <<__LateInit>>
  private AliasCT117 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT117 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT117 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT117>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT117>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT117 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT117> {
    return vec[#A,EC#B,new AClass()];
  }
}
case type CT118 = HH\FunctionRef<(function(): void)>|AClass;
              type AliasCT118 = CT118;

  
class CheckAliasCT118<T as AliasCT118> extends BaseCheck {
  const type T = AliasCT118;
  const string NAME = 'AliasCT118';

  <<__LateInit>>
  private AliasCT118 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT118 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT118 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT118>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT118>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT118 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT118> {
    return vec[my_func<>,new AClass()];
  }
}
case type CT119 = HH\MemberOf<EC, float>|AClass;
              type AliasCT119 = CT119;

  
class CheckAliasCT119<T as AliasCT119> extends BaseCheck {
  const type T = AliasCT119;
  const string NAME = 'AliasCT119';

  <<__LateInit>>
  private AliasCT119 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT119 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT119 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT119>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT119>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT119 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT119> {
    return vec[EC::A,EC::B,new AClass()];
  }
}
case type CT120 = I|AClass;
              type AliasCT120 = CT120;

  
class CheckAliasCT120<T as AliasCT120> extends BaseCheck {
  const type T = AliasCT120;
  const string NAME = 'AliasCT120';

  <<__LateInit>>
  private AliasCT120 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT120 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT120 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT120>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT120>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT120 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT120> {
    return vec[new AClass(),new InstanceOfI()];
  }
}
case type CT121 = KeyedContainer<arraykey, mixed>|AClass;
              type AliasCT121 = CT121;

  
class CheckAliasCT121<T as AliasCT121> extends BaseCheck {
  const type T = AliasCT121;
  const string NAME = 'AliasCT121';

  <<__LateInit>>
  private AliasCT121 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT121 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT121 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT121>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT121>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT121 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT121> {
    return vec[dict[],new AClass(),vec[]];
  }
}
case type CT122 = KeyedTraversable<arraykey, mixed>|AClass;
              type AliasCT122 = CT122;

  
class CheckAliasCT122<T as AliasCT122> extends BaseCheck {
  const type T = AliasCT122;
  const string NAME = 'AliasCT122';

  <<__LateInit>>
  private AliasCT122 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT122 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT122 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT122>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT122>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT122 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT122> {
    return vec[dict[],keyset[],new AClass(),vec[]];
  }
}
case type CT123 = MyTrait|AClass;
              type AliasCT123 = CT123;

  
class CheckAliasCT123<T as AliasCT123> extends BaseCheck {
  const type T = AliasCT123;
  const string NAME = 'AliasCT123';

  <<__LateInit>>
  private AliasCT123 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT123 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT123 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT123>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT123>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT123 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT123> {
    return vec[new AClass()];
  }
}
case type CT124 = ReifiedClass<null>|AClass;
              type AliasCT124 = CT124;

  
class CheckAliasCT124<T as AliasCT124> extends BaseCheck {
  const type T = AliasCT124;
  const string NAME = 'AliasCT124';

  <<__LateInit>>
  private AliasCT124 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT124 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT124 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT124>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT124>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT124 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT124> {
    return vec[new AClass(),new ReifiedClass<null>()];
  }
}
case type CT125 = Stringish|AClass;
              type AliasCT125 = CT125;

  
class CheckAliasCT125<T as AliasCT125> extends BaseCheck {
  const type T = AliasCT125;
  const string NAME = 'AliasCT125';

  <<__LateInit>>
  private AliasCT125 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT125 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT125 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT125>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT125>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT125 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT125> {
    return vec['','hello world',new AClass(),new StringishObj()];
  }
}
case type CT126 = Traversable<mixed>|AClass;
              type AliasCT126 = CT126;

  
class CheckAliasCT126<T as AliasCT126> extends BaseCheck {
  const type T = AliasCT126;
  const string NAME = 'AliasCT126';

  <<__LateInit>>
  private AliasCT126 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT126 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT126 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT126>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT126>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT126 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT126> {
    return vec[dict[],keyset[],new AClass(),vec[]];
  }
}
case type CT127 = XHPChild|AClass;
              type AliasCT127 = CT127;

  
class CheckAliasCT127<T as AliasCT127> extends BaseCheck {
  const type T = AliasCT127;
  const string NAME = 'AliasCT127';

  <<__LateInit>>
  private AliasCT127 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT127 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT127 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT127>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT127>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT127 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT127> {
    return vec['','hello world',0,1,<my-xhp/>,new AClass()];
  }
}
case type CT128 = arraykey|AClass;
              type AliasCT128 = CT128;

  
class CheckAliasCT128<T as AliasCT128> extends BaseCheck {
  const type T = AliasCT128;
  const string NAME = 'AliasCT128';

  <<__LateInit>>
  private AliasCT128 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT128 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT128 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT128>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT128>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT128 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT128> {
    return vec['','hello world',0,1,new AClass()];
  }
}
case type CT129 = bool|AClass;
              type AliasCT129 = CT129;

  
class CheckAliasCT129<T as AliasCT129> extends BaseCheck {
  const type T = AliasCT129;
  const string NAME = 'AliasCT129';

  <<__LateInit>>
  private AliasCT129 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT129 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT129 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT129>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT129>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT129 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT129> {
    return vec[false,new AClass(),true];
  }
}
case type CT130 = dict<arraykey, mixed>|AClass;
              type AliasCT130 = CT130;

  
class CheckAliasCT130<T as AliasCT130> extends BaseCheck {
  const type T = AliasCT130;
  const string NAME = 'AliasCT130';

  <<__LateInit>>
  private AliasCT130 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT130 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT130 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT130>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT130>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT130 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT130> {
    return vec[dict[],new AClass()];
  }
}
case type CT131 = dynamic|AClass;
              type AliasCT131 = CT131;

  
class CheckAliasCT131<T as AliasCT131> extends BaseCheck {
  const type T = AliasCT131;
  const string NAME = 'AliasCT131';

  <<__LateInit>>
  private AliasCT131 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT131 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT131 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT131>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT131>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT131 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT131> {
    return vec[false,new AClass(),null,shape('x' => 10),shape(),true];
  }
}
case type CT132 = float|AClass;
              type AliasCT132 = CT132;

  
class CheckAliasCT132<T as AliasCT132> extends BaseCheck {
  const type T = AliasCT132;
  const string NAME = 'AliasCT132';

  <<__LateInit>>
  private AliasCT132 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT132 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT132 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT132>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT132>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT132 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT132> {
    return vec[0.0,3.14,new AClass()];
  }
}
case type CT133 = int|AClass;
              type AliasCT133 = CT133;

  
class CheckAliasCT133<T as AliasCT133> extends BaseCheck {
  const type T = AliasCT133;
  const string NAME = 'AliasCT133';

  <<__LateInit>>
  private AliasCT133 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT133 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT133 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT133>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT133>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT133 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT133> {
    return vec[0,1,new AClass()];
  }
}
case type CT134 = keyset<arraykey>|AClass;
              type AliasCT134 = CT134;

  
class CheckAliasCT134<T as AliasCT134> extends BaseCheck {
  const type T = AliasCT134;
  const string NAME = 'AliasCT134';

  <<__LateInit>>
  private AliasCT134 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT134 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT134 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT134>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT134>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT134 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT134> {
    return vec[keyset[],new AClass()];
  }
}
case type CT135 = mixed|AClass;
              type AliasCT135 = CT135;

  
class CheckAliasCT135<T as AliasCT135> extends BaseCheck {
  const type T = AliasCT135;
  const string NAME = 'AliasCT135';

  <<__LateInit>>
  private AliasCT135 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT135 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT135 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT135>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT135>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT135 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT135> {
    return vec['','hello world',0,1,false,new AClass(),null,true];
  }
}
case type CT136 = nonnull|AClass;
              type AliasCT136 = CT136;

  
class CheckAliasCT136<T as AliasCT136> extends BaseCheck {
  const type T = AliasCT136;
  const string NAME = 'AliasCT136';

  <<__LateInit>>
  private AliasCT136 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT136 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT136 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT136>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT136>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT136 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT136> {
    return vec['','hello world',0,1,false,new AClass(),true];
  }
}
case type CT137 = noreturn|AClass;
              type AliasCT137 = CT137;

  
class CheckAliasCT137<T as AliasCT137> extends BaseCheck {
  const type T = AliasCT137;
  const string NAME = 'AliasCT137';

  <<__LateInit>>
  private AliasCT137 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT137 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT137 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT137>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT137>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT137 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT137> {
    return vec[new AClass()];
  }
}
case type CT138 = nothing|AClass;
              type AliasCT138 = CT138;

  
class CheckAliasCT138<T as AliasCT138> extends BaseCheck {
  const type T = AliasCT138;
  const string NAME = 'AliasCT138';

  <<__LateInit>>
  private AliasCT138 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT138 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT138 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT138>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT138>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT138 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT138> {
    return vec[new AClass()];
  }
}
case type CT139 = null|AClass;
              type AliasCT139 = CT139;

  
class CheckAliasCT139<T as AliasCT139> extends BaseCheck {
  const type T = AliasCT139;
  const string NAME = 'AliasCT139';

  <<__LateInit>>
  private AliasCT139 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT139 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT139 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT139>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT139>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT139 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT139> {
    return vec[new AClass(),null];
  }
}
case type CT140 = num|AClass;
              type AliasCT140 = CT140;

  
class CheckAliasCT140<T as AliasCT140> extends BaseCheck {
  const type T = AliasCT140;
  const string NAME = 'AliasCT140';

  <<__LateInit>>
  private AliasCT140 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT140 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT140 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT140>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT140>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT140 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT140> {
    return vec[0,0.0,1,3.14,new AClass()];
  }
}
case type CT141 = resource|AClass;
              type AliasCT141 = CT141;

  
class CheckAliasCT141<T as AliasCT141> extends BaseCheck {
  const type T = AliasCT141;
  const string NAME = 'AliasCT141';

  <<__LateInit>>
  private AliasCT141 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT141 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT141 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT141>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT141>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT141 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT141> {
    return vec[imagecreate(10, 10),new AClass()];
  }
}
case type CT142 = shape(...)|AClass;
              type AliasCT142 = CT142;

  
class CheckAliasCT142<T as AliasCT142> extends BaseCheck {
  const type T = AliasCT142;
  const string NAME = 'AliasCT142';

  <<__LateInit>>
  private AliasCT142 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT142 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT142 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT142>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT142>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT142 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT142> {
    return vec[new AClass(),shape('x' => 10),shape()];
  }
}
case type CT143 = string|AClass;
              type AliasCT143 = CT143;

  
class CheckAliasCT143<T as AliasCT143> extends BaseCheck {
  const type T = AliasCT143;
  const string NAME = 'AliasCT143';

  <<__LateInit>>
  private AliasCT143 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT143 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT143 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT143>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT143>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT143 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT143> {
    return vec['','hello world',new AClass()];
  }
}
case type CT144 = vec<mixed>|AClass;
              type AliasCT144 = CT144;

  
class CheckAliasCT144<T as AliasCT144> extends BaseCheck {
  const type T = AliasCT144;
  const string NAME = 'AliasCT144';

  <<__LateInit>>
  private AliasCT144 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT144 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT144 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT144>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT144>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT144 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT144> {
    return vec[new AClass(),vec[]];
  }
}
case type CT145 = vec_or_dict<string>|AClass;
              type AliasCT145 = CT145;

  
class CheckAliasCT145<T as AliasCT145> extends BaseCheck {
  const type T = AliasCT145;
  const string NAME = 'AliasCT145';

  <<__LateInit>>
  private AliasCT145 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT145 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT145 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT145>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT145>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT145 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT145> {
    return vec[dict[],new AClass(),vec[]];
  }
}
case type CT146 = void|AClass;
              type AliasCT146 = CT146;

  
class CheckAliasCT146<T as AliasCT146> extends BaseCheck {
  const type T = AliasCT146;
  const string NAME = 'AliasCT146';

  <<__LateInit>>
  private AliasCT146 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT146 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT146 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT146>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT146>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT146 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT146> {
    return vec[new AClass(),null];
  }
}
case type CT147 = AbsFinal|AbsFinal;
              type AliasCT147 = CT147;

  
class CheckAliasCT147<T as AliasCT147> extends BaseCheck {
  const type T = AliasCT147;
  const string NAME = 'AliasCT147';

  <<__LateInit>>
  private AliasCT147 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT147 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT147 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT147>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT147>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT147 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT147> {
    return vec[];
  }
}
case type CT148 = Awaitable<num>|AbsFinal;
              type AliasCT148 = CT148;

  
class CheckAliasCT148<T as AliasCT148> extends BaseCheck {
  const type T = AliasCT148;
  const string NAME = 'AliasCT148';

  <<__LateInit>>
  private AliasCT148 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT148 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT148 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT148>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT148>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT148 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT148> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT149 = Container<mixed>|AbsFinal;
              type AliasCT149 = CT149;

  
class CheckAliasCT149<T as AliasCT149> extends BaseCheck {
  const type T = AliasCT149;
  const string NAME = 'AliasCT149';

  <<__LateInit>>
  private AliasCT149 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT149 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT149 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT149>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT149>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT149 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT149> {
    return vec[keyset[],vec[]];
  }
}
case type CT150 = HH\AnyArray<arraykey, mixed>|AbsFinal;
              type AliasCT150 = CT150;

  
class CheckAliasCT150<T as AliasCT150> extends BaseCheck {
  const type T = AliasCT150;
  const string NAME = 'AliasCT150';

  <<__LateInit>>
  private AliasCT150 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT150 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT150 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT150>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT150>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT150 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT150> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT151 = HH\EnumClass\Label<EC, float>|AbsFinal;
              type AliasCT151 = CT151;

  
class CheckAliasCT151<T as AliasCT151> extends BaseCheck {
  const type T = AliasCT151;
  const string NAME = 'AliasCT151';

  <<__LateInit>>
  private AliasCT151 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT151 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT151 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT151>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT151>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT151 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT151> {
    return vec[#A,EC#B];
  }
}
case type CT152 = HH\FunctionRef<(function(): void)>|AbsFinal;
              type AliasCT152 = CT152;

  
class CheckAliasCT152<T as AliasCT152> extends BaseCheck {
  const type T = AliasCT152;
  const string NAME = 'AliasCT152';

  <<__LateInit>>
  private AliasCT152 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT152 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT152 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT152>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT152>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT152 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT152> {
    return vec[my_func<>];
  }
}
case type CT153 = HH\MemberOf<EC, float>|AbsFinal;
              type AliasCT153 = CT153;

  
class CheckAliasCT153<T as AliasCT153> extends BaseCheck {
  const type T = AliasCT153;
  const string NAME = 'AliasCT153';

  <<__LateInit>>
  private AliasCT153 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT153 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT153 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT153>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT153>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT153 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT153> {
    return vec[EC::A,EC::B];
  }
}
case type CT154 = I|AbsFinal;
              type AliasCT154 = CT154;

  
class CheckAliasCT154<T as AliasCT154> extends BaseCheck {
  const type T = AliasCT154;
  const string NAME = 'AliasCT154';

  <<__LateInit>>
  private AliasCT154 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT154 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT154 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT154>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT154>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT154 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT154> {
    return vec[new InstanceOfI()];
  }
}
case type CT155 = KeyedContainer<arraykey, mixed>|AbsFinal;
              type AliasCT155 = CT155;

  
class CheckAliasCT155<T as AliasCT155> extends BaseCheck {
  const type T = AliasCT155;
  const string NAME = 'AliasCT155';

  <<__LateInit>>
  private AliasCT155 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT155 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT155 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT155>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT155>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT155 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT155> {
    return vec[dict[],vec[]];
  }
}
case type CT156 = KeyedTraversable<arraykey, mixed>|AbsFinal;
              type AliasCT156 = CT156;

  
class CheckAliasCT156<T as AliasCT156> extends BaseCheck {
  const type T = AliasCT156;
  const string NAME = 'AliasCT156';

  <<__LateInit>>
  private AliasCT156 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT156 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT156 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT156>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT156>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT156 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT156> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT157 = MyTrait|AbsFinal;
              type AliasCT157 = CT157;

  
class CheckAliasCT157<T as AliasCT157> extends BaseCheck {
  const type T = AliasCT157;
  const string NAME = 'AliasCT157';

  <<__LateInit>>
  private AliasCT157 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT157 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT157 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT157>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT157>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT157 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT157> {
    return vec[];
  }
}
case type CT158 = ReifiedClass<null>|AbsFinal;
              type AliasCT158 = CT158;

  
class CheckAliasCT158<T as AliasCT158> extends BaseCheck {
  const type T = AliasCT158;
  const string NAME = 'AliasCT158';

  <<__LateInit>>
  private AliasCT158 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT158 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT158 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT158>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT158>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT158 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT158> {
    return vec[new ReifiedClass<null>()];
  }
}
case type CT159 = Stringish|AbsFinal;
              type AliasCT159 = CT159;

  
class CheckAliasCT159<T as AliasCT159> extends BaseCheck {
  const type T = AliasCT159;
  const string NAME = 'AliasCT159';

  <<__LateInit>>
  private AliasCT159 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT159 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT159 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT159>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT159>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT159 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT159> {
    return vec['','hello world',new StringishObj()];
  }
}
case type CT160 = Traversable<mixed>|AbsFinal;
              type AliasCT160 = CT160;

  
class CheckAliasCT160<T as AliasCT160> extends BaseCheck {
  const type T = AliasCT160;
  const string NAME = 'AliasCT160';

  <<__LateInit>>
  private AliasCT160 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT160 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT160 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT160>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT160>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT160 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT160> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT161 = XHPChild|AbsFinal;
              type AliasCT161 = CT161;

  
class CheckAliasCT161<T as AliasCT161> extends BaseCheck {
  const type T = AliasCT161;
  const string NAME = 'AliasCT161';

  <<__LateInit>>
  private AliasCT161 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT161 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT161 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT161>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT161>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT161 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT161> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT162 = arraykey|AbsFinal;
              type AliasCT162 = CT162;

  
class CheckAliasCT162<T as AliasCT162> extends BaseCheck {
  const type T = AliasCT162;
  const string NAME = 'AliasCT162';

  <<__LateInit>>
  private AliasCT162 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT162 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT162 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT162>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT162>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT162 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT162> {
    return vec['','hello world',0,1];
  }
}
case type CT163 = bool|AbsFinal;
              type AliasCT163 = CT163;

  
class CheckAliasCT163<T as AliasCT163> extends BaseCheck {
  const type T = AliasCT163;
  const string NAME = 'AliasCT163';

  <<__LateInit>>
  private AliasCT163 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT163 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT163 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT163>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT163>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT163 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT163> {
    return vec[false,true];
  }
}
case type CT164 = dict<arraykey, mixed>|AbsFinal;
              type AliasCT164 = CT164;

  
class CheckAliasCT164<T as AliasCT164> extends BaseCheck {
  const type T = AliasCT164;
  const string NAME = 'AliasCT164';

  <<__LateInit>>
  private AliasCT164 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT164 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT164 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT164>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT164>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT164 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT164> {
    return vec[dict[]];
  }
}
case type CT165 = dynamic|AbsFinal;
              type AliasCT165 = CT165;

  
class CheckAliasCT165<T as AliasCT165> extends BaseCheck {
  const type T = AliasCT165;
  const string NAME = 'AliasCT165';

  <<__LateInit>>
  private AliasCT165 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT165 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT165 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT165>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT165>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT165 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT165> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT166 = float|AbsFinal;
              type AliasCT166 = CT166;

  
class CheckAliasCT166<T as AliasCT166> extends BaseCheck {
  const type T = AliasCT166;
  const string NAME = 'AliasCT166';

  <<__LateInit>>
  private AliasCT166 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT166 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT166 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT166>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT166>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT166 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT166> {
    return vec[0.0,3.14];
  }
}
case type CT167 = int|AbsFinal;
              type AliasCT167 = CT167;

  
class CheckAliasCT167<T as AliasCT167> extends BaseCheck {
  const type T = AliasCT167;
  const string NAME = 'AliasCT167';

  <<__LateInit>>
  private AliasCT167 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT167 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT167 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT167>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT167>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT167 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT167> {
    return vec[0,1];
  }
}
case type CT168 = keyset<arraykey>|AbsFinal;
              type AliasCT168 = CT168;

  
class CheckAliasCT168<T as AliasCT168> extends BaseCheck {
  const type T = AliasCT168;
  const string NAME = 'AliasCT168';

  <<__LateInit>>
  private AliasCT168 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT168 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT168 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT168>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT168>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT168 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT168> {
    return vec[keyset[]];
  }
}
case type CT169 = mixed|AbsFinal;
              type AliasCT169 = CT169;

  
class CheckAliasCT169<T as AliasCT169> extends BaseCheck {
  const type T = AliasCT169;
  const string NAME = 'AliasCT169';

  <<__LateInit>>
  private AliasCT169 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT169 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT169 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT169>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT169>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT169 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT169> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT170 = nonnull|AbsFinal;
              type AliasCT170 = CT170;

  
class CheckAliasCT170<T as AliasCT170> extends BaseCheck {
  const type T = AliasCT170;
  const string NAME = 'AliasCT170';

  <<__LateInit>>
  private AliasCT170 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT170 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT170 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT170>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT170>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT170 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT170> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT171 = noreturn|AbsFinal;
              type AliasCT171 = CT171;

  
class CheckAliasCT171<T as AliasCT171> extends BaseCheck {
  const type T = AliasCT171;
  const string NAME = 'AliasCT171';

  <<__LateInit>>
  private AliasCT171 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT171 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT171 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT171>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT171>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT171 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT171> {
    return vec[];
  }
}
case type CT172 = nothing|AbsFinal;
              type AliasCT172 = CT172;

  
class CheckAliasCT172<T as AliasCT172> extends BaseCheck {
  const type T = AliasCT172;
  const string NAME = 'AliasCT172';

  <<__LateInit>>
  private AliasCT172 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT172 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT172 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT172>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT172>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT172 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT172> {
    return vec[];
  }
}
case type CT173 = null|AbsFinal;
              type AliasCT173 = CT173;

  
class CheckAliasCT173<T as AliasCT173> extends BaseCheck {
  const type T = AliasCT173;
  const string NAME = 'AliasCT173';

  <<__LateInit>>
  private AliasCT173 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT173 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT173 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT173>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT173>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT173 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT173> {
    return vec[null];
  }
}
case type CT174 = num|AbsFinal;
              type AliasCT174 = CT174;

  
class CheckAliasCT174<T as AliasCT174> extends BaseCheck {
  const type T = AliasCT174;
  const string NAME = 'AliasCT174';

  <<__LateInit>>
  private AliasCT174 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT174 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT174 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT174>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT174>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT174 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT174> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT175 = resource|AbsFinal;
              type AliasCT175 = CT175;

  
class CheckAliasCT175<T as AliasCT175> extends BaseCheck {
  const type T = AliasCT175;
  const string NAME = 'AliasCT175';

  <<__LateInit>>
  private AliasCT175 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT175 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT175 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT175>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT175>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT175 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT175> {
    return vec[imagecreate(10, 10)];
  }
}
case type CT176 = shape(...)|AbsFinal;
              type AliasCT176 = CT176;

  
class CheckAliasCT176<T as AliasCT176> extends BaseCheck {
  const type T = AliasCT176;
  const string NAME = 'AliasCT176';

  <<__LateInit>>
  private AliasCT176 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT176 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT176 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT176>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT176>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT176 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT176> {
    return vec[shape('x' => 10),shape()];
  }
}
case type CT177 = string|AbsFinal;
              type AliasCT177 = CT177;

  
class CheckAliasCT177<T as AliasCT177> extends BaseCheck {
  const type T = AliasCT177;
  const string NAME = 'AliasCT177';

  <<__LateInit>>
  private AliasCT177 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT177 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT177 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT177>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT177>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT177 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT177> {
    return vec['','hello world'];
  }
}
case type CT178 = vec<mixed>|AbsFinal;
              type AliasCT178 = CT178;

  
class CheckAliasCT178<T as AliasCT178> extends BaseCheck {
  const type T = AliasCT178;
  const string NAME = 'AliasCT178';

  <<__LateInit>>
  private AliasCT178 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT178 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT178 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT178>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT178>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT178 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT178> {
    return vec[vec[]];
  }
}
case type CT179 = vec_or_dict<string>|AbsFinal;
              type AliasCT179 = CT179;

  
class CheckAliasCT179<T as AliasCT179> extends BaseCheck {
  const type T = AliasCT179;
  const string NAME = 'AliasCT179';

  <<__LateInit>>
  private AliasCT179 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT179 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT179 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT179>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT179>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT179 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT179> {
    return vec[dict[],vec[]];
  }
}
case type CT180 = void|AbsFinal;
              type AliasCT180 = CT180;

  
class CheckAliasCT180<T as AliasCT180> extends BaseCheck {
  const type T = AliasCT180;
  const string NAME = 'AliasCT180';

  <<__LateInit>>
  private AliasCT180 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT180 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT180 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT180>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT180>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT180 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT180> {
    return vec[null];
  }
}
case type CT181 = Awaitable<num>|Awaitable<num>;
              type AliasCT181 = CT181;

  
class CheckAliasCT181<T as AliasCT181> extends BaseCheck {
  const type T = AliasCT181;
  const string NAME = 'AliasCT181';

  <<__LateInit>>
  private AliasCT181 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT181 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT181 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT181>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT181>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT181 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT181> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT182 = Container<mixed>|Awaitable<num>;
              type AliasCT182 = CT182;

  
class CheckAliasCT182<T as AliasCT182> extends BaseCheck {
  const type T = AliasCT182;
  const string NAME = 'AliasCT182';

  <<__LateInit>>
  private AliasCT182 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT182 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT182 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT182>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT182>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT182 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT182> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },keyset[],vec[]];
  }
}
case type CT183 = HH\AnyArray<arraykey, mixed>|Awaitable<num>;
              type AliasCT183 = CT183;

  
class CheckAliasCT183<T as AliasCT183> extends BaseCheck {
  const type T = AliasCT183;
  const string NAME = 'AliasCT183';

  <<__LateInit>>
  private AliasCT183 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT183 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT183 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT183>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT183>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT183 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT183> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },dict[],keyset[],vec[]];
  }
}
case type CT184 = HH\EnumClass\Label<EC, float>|Awaitable<num>;
              type AliasCT184 = CT184;

  
class CheckAliasCT184<T as AliasCT184> extends BaseCheck {
  const type T = AliasCT184;
  const string NAME = 'AliasCT184';

  <<__LateInit>>
  private AliasCT184 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT184 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT184 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT184>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT184>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT184 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT184> {
    return vec[#A,EC#B,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT185 = HH\FunctionRef<(function(): void)>|Awaitable<num>;
              type AliasCT185 = CT185;

  
class CheckAliasCT185<T as AliasCT185> extends BaseCheck {
  const type T = AliasCT185;
  const string NAME = 'AliasCT185';

  <<__LateInit>>
  private AliasCT185 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT185 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT185 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT185>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT185>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT185 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT185> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },my_func<>];
  }
}
case type CT186 = HH\MemberOf<EC, float>|Awaitable<num>;
              type AliasCT186 = CT186;

  
class CheckAliasCT186<T as AliasCT186> extends BaseCheck {
  const type T = AliasCT186;
  const string NAME = 'AliasCT186';

  <<__LateInit>>
  private AliasCT186 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT186 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT186 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT186>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT186>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT186 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT186> {
    return vec[EC::A,EC::B,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT187 = I|Awaitable<num>;
              type AliasCT187 = CT187;

  
class CheckAliasCT187<T as AliasCT187> extends BaseCheck {
  const type T = AliasCT187;
  const string NAME = 'AliasCT187';

  <<__LateInit>>
  private AliasCT187 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT187 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT187 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT187>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT187>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT187 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT187> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },new InstanceOfI()];
  }
}
case type CT188 = KeyedContainer<arraykey, mixed>|Awaitable<num>;
              type AliasCT188 = CT188;

  
class CheckAliasCT188<T as AliasCT188> extends BaseCheck {
  const type T = AliasCT188;
  const string NAME = 'AliasCT188';

  <<__LateInit>>
  private AliasCT188 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT188 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT188 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT188>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT188>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT188 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT188> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },dict[],vec[]];
  }
}
case type CT189 = KeyedTraversable<arraykey, mixed>|Awaitable<num>;
              type AliasCT189 = CT189;

  
class CheckAliasCT189<T as AliasCT189> extends BaseCheck {
  const type T = AliasCT189;
  const string NAME = 'AliasCT189';

  <<__LateInit>>
  private AliasCT189 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT189 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT189 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT189>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT189>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT189 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT189> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },dict[],keyset[],vec[]];
  }
}
case type CT190 = MyTrait|Awaitable<num>;
              type AliasCT190 = CT190;

  
class CheckAliasCT190<T as AliasCT190> extends BaseCheck {
  const type T = AliasCT190;
  const string NAME = 'AliasCT190';

  <<__LateInit>>
  private AliasCT190 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT190 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT190 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT190>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT190>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT190 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT190> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT191 = ReifiedClass<null>|Awaitable<num>;
              type AliasCT191 = CT191;

  
class CheckAliasCT191<T as AliasCT191> extends BaseCheck {
  const type T = AliasCT191;
  const string NAME = 'AliasCT191';

  <<__LateInit>>
  private AliasCT191 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT191 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT191 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT191>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT191>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT191 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT191> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },new ReifiedClass<null>()];
  }
}
case type CT192 = Stringish|Awaitable<num>;
              type AliasCT192 = CT192;

  
class CheckAliasCT192<T as AliasCT192> extends BaseCheck {
  const type T = AliasCT192;
  const string NAME = 'AliasCT192';

  <<__LateInit>>
  private AliasCT192 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT192 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT192 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT192>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT192>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT192 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT192> {
    return vec['','hello world',async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },new StringishObj()];
  }
}
case type CT193 = Traversable<mixed>|Awaitable<num>;
              type AliasCT193 = CT193;

  
class CheckAliasCT193<T as AliasCT193> extends BaseCheck {
  const type T = AliasCT193;
  const string NAME = 'AliasCT193';

  <<__LateInit>>
  private AliasCT193 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT193 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT193 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT193>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT193>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT193 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT193> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },dict[],keyset[],vec[]];
  }
}
case type CT194 = XHPChild|Awaitable<num>;
              type AliasCT194 = CT194;

  
class CheckAliasCT194<T as AliasCT194> extends BaseCheck {
  const type T = AliasCT194;
  const string NAME = 'AliasCT194';

  <<__LateInit>>
  private AliasCT194 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT194 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT194 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT194>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT194>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT194 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT194> {
    return vec['','hello world',0,1,<my-xhp/>,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT195 = arraykey|Awaitable<num>;
              type AliasCT195 = CT195;

  
class CheckAliasCT195<T as AliasCT195> extends BaseCheck {
  const type T = AliasCT195;
  const string NAME = 'AliasCT195';

  <<__LateInit>>
  private AliasCT195 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT195 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT195 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT195>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT195>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT195 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT195> {
    return vec['','hello world',0,1,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT196 = bool|Awaitable<num>;
              type AliasCT196 = CT196;

  
class CheckAliasCT196<T as AliasCT196> extends BaseCheck {
  const type T = AliasCT196;
  const string NAME = 'AliasCT196';

  <<__LateInit>>
  private AliasCT196 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT196 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT196 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT196>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT196>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT196 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT196> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },false,true];
  }
}
case type CT197 = dict<arraykey, mixed>|Awaitable<num>;
              type AliasCT197 = CT197;

  
class CheckAliasCT197<T as AliasCT197> extends BaseCheck {
  const type T = AliasCT197;
  const string NAME = 'AliasCT197';

  <<__LateInit>>
  private AliasCT197 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT197 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT197 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT197>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT197>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT197 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT197> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },dict[]];
  }
}
case type CT198 = dynamic|Awaitable<num>;
              type AliasCT198 = CT198;

  
class CheckAliasCT198<T as AliasCT198> extends BaseCheck {
  const type T = AliasCT198;
  const string NAME = 'AliasCT198';

  <<__LateInit>>
  private AliasCT198 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT198 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT198 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT198>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT198>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT198 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT198> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },false,null,shape('x' => 10),shape(),true];
  }
}
case type CT199 = float|Awaitable<num>;
              type AliasCT199 = CT199;

  
class CheckAliasCT199<T as AliasCT199> extends BaseCheck {
  const type T = AliasCT199;
  const string NAME = 'AliasCT199';

  <<__LateInit>>
  private AliasCT199 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT199 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT199 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT199>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT199>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT199 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT199> {
    return vec[0.0,3.14,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT200 = int|Awaitable<num>;
              type AliasCT200 = CT200;

  
class CheckAliasCT200<T as AliasCT200> extends BaseCheck {
  const type T = AliasCT200;
  const string NAME = 'AliasCT200';

  <<__LateInit>>
  private AliasCT200 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT200 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT200 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT200>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT200>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT200 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT200> {
    return vec[0,1,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT201 = keyset<arraykey>|Awaitable<num>;
              type AliasCT201 = CT201;

  
class CheckAliasCT201<T as AliasCT201> extends BaseCheck {
  const type T = AliasCT201;
  const string NAME = 'AliasCT201';

  <<__LateInit>>
  private AliasCT201 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT201 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT201 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT201>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT201>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT201 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT201> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },keyset[]];
  }
}
case type CT202 = mixed|Awaitable<num>;
              type AliasCT202 = CT202;

  
class CheckAliasCT202<T as AliasCT202> extends BaseCheck {
  const type T = AliasCT202;
  const string NAME = 'AliasCT202';

  <<__LateInit>>
  private AliasCT202 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT202 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT202 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT202>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT202>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT202 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT202> {
    return vec['','hello world',0,1,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },false,null,true];
  }
}
case type CT203 = nonnull|Awaitable<num>;
              type AliasCT203 = CT203;

  
class CheckAliasCT203<T as AliasCT203> extends BaseCheck {
  const type T = AliasCT203;
  const string NAME = 'AliasCT203';

  <<__LateInit>>
  private AliasCT203 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT203 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT203 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT203>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT203>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT203 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT203> {
    return vec['','hello world',0,1,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },false,true];
  }
}
case type CT204 = noreturn|Awaitable<num>;
              type AliasCT204 = CT204;

  
class CheckAliasCT204<T as AliasCT204> extends BaseCheck {
  const type T = AliasCT204;
  const string NAME = 'AliasCT204';

  <<__LateInit>>
  private AliasCT204 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT204 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT204 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT204>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT204>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT204 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT204> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT205 = nothing|Awaitable<num>;
              type AliasCT205 = CT205;

  
class CheckAliasCT205<T as AliasCT205> extends BaseCheck {
  const type T = AliasCT205;
  const string NAME = 'AliasCT205';

  <<__LateInit>>
  private AliasCT205 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT205 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT205 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT205>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT205>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT205 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT205> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT206 = null|Awaitable<num>;
              type AliasCT206 = CT206;

  
class CheckAliasCT206<T as AliasCT206> extends BaseCheck {
  const type T = AliasCT206;
  const string NAME = 'AliasCT206';

  <<__LateInit>>
  private AliasCT206 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT206 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT206 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT206>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT206>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT206 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT206> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },null];
  }
}
case type CT207 = num|Awaitable<num>;
              type AliasCT207 = CT207;

  
class CheckAliasCT207<T as AliasCT207> extends BaseCheck {
  const type T = AliasCT207;
  const string NAME = 'AliasCT207';

  <<__LateInit>>
  private AliasCT207 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT207 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT207 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT207>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT207>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT207 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT207> {
    return vec[0,0.0,1,3.14,async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT208 = resource|Awaitable<num>;
              type AliasCT208 = CT208;

  
class CheckAliasCT208<T as AliasCT208> extends BaseCheck {
  const type T = AliasCT208;
  const string NAME = 'AliasCT208';

  <<__LateInit>>
  private AliasCT208 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT208 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT208 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT208>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT208>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT208 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT208> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },imagecreate(10, 10)];
  }
}
case type CT209 = shape(...)|Awaitable<num>;
              type AliasCT209 = CT209;

  
class CheckAliasCT209<T as AliasCT209> extends BaseCheck {
  const type T = AliasCT209;
  const string NAME = 'AliasCT209';

  <<__LateInit>>
  private AliasCT209 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT209 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT209 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT209>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT209>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT209 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT209> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },shape('x' => 10),shape()];
  }
}
case type CT210 = string|Awaitable<num>;
              type AliasCT210 = CT210;

  
class CheckAliasCT210<T as AliasCT210> extends BaseCheck {
  const type T = AliasCT210;
  const string NAME = 'AliasCT210';

  <<__LateInit>>
  private AliasCT210 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT210 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT210 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT210>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT210>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT210 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT210> {
    return vec['','hello world',async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; }];
  }
}
case type CT211 = vec<mixed>|Awaitable<num>;
              type AliasCT211 = CT211;

  
class CheckAliasCT211<T as AliasCT211> extends BaseCheck {
  const type T = AliasCT211;
  const string NAME = 'AliasCT211';

  <<__LateInit>>
  private AliasCT211 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT211 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT211 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT211>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT211>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT211 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT211> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },vec[]];
  }
}
case type CT212 = vec_or_dict<string>|Awaitable<num>;
              type AliasCT212 = CT212;

  
class CheckAliasCT212<T as AliasCT212> extends BaseCheck {
  const type T = AliasCT212;
  const string NAME = 'AliasCT212';

  <<__LateInit>>
  private AliasCT212 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT212 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT212 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT212>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT212>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT212 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT212> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },dict[],vec[]];
  }
}
case type CT213 = void|Awaitable<num>;
              type AliasCT213 = CT213;

  
class CheckAliasCT213<T as AliasCT213> extends BaseCheck {
  const type T = AliasCT213;
  const string NAME = 'AliasCT213';

  <<__LateInit>>
  private AliasCT213 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT213 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT213 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT213>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT213>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT213 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT213> {
    return vec[async { return 0.0; },async { return 0; },async { return 1; },async { return 3.14; },null];
  }
}
case type CT214 = Container<mixed>|Container<mixed>;
              type AliasCT214 = CT214;

  
class CheckAliasCT214<T as AliasCT214> extends BaseCheck {
  const type T = AliasCT214;
  const string NAME = 'AliasCT214';

  <<__LateInit>>
  private AliasCT214 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT214 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT214 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT214>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT214>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT214 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT214> {
    return vec[keyset[],vec[]];
  }
}
case type CT215 = HH\AnyArray<arraykey, mixed>|Container<mixed>;
              type AliasCT215 = CT215;

  
class CheckAliasCT215<T as AliasCT215> extends BaseCheck {
  const type T = AliasCT215;
  const string NAME = 'AliasCT215';

  <<__LateInit>>
  private AliasCT215 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT215 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT215 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT215>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT215>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT215 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT215> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT216 = HH\EnumClass\Label<EC, float>|Container<mixed>;
              type AliasCT216 = CT216;

  
class CheckAliasCT216<T as AliasCT216> extends BaseCheck {
  const type T = AliasCT216;
  const string NAME = 'AliasCT216';

  <<__LateInit>>
  private AliasCT216 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT216 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT216 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT216>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT216>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT216 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT216> {
    return vec[#A,EC#B,keyset[],vec[]];
  }
}
case type CT217 = HH\FunctionRef<(function(): void)>|Container<mixed>;
              type AliasCT217 = CT217;

  
class CheckAliasCT217<T as AliasCT217> extends BaseCheck {
  const type T = AliasCT217;
  const string NAME = 'AliasCT217';

  <<__LateInit>>
  private AliasCT217 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT217 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT217 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT217>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT217>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT217 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT217> {
    return vec[keyset[],my_func<>,vec[]];
  }
}
case type CT218 = HH\MemberOf<EC, float>|Container<mixed>;
              type AliasCT218 = CT218;

  
class CheckAliasCT218<T as AliasCT218> extends BaseCheck {
  const type T = AliasCT218;
  const string NAME = 'AliasCT218';

  <<__LateInit>>
  private AliasCT218 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT218 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT218 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT218>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT218>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT218 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT218> {
    return vec[EC::A,EC::B,keyset[],vec[]];
  }
}
case type CT219 = I|Container<mixed>;
              type AliasCT219 = CT219;

  
class CheckAliasCT219<T as AliasCT219> extends BaseCheck {
  const type T = AliasCT219;
  const string NAME = 'AliasCT219';

  <<__LateInit>>
  private AliasCT219 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT219 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT219 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT219>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT219>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT219 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT219> {
    return vec[keyset[],new InstanceOfI(),vec[]];
  }
}
case type CT220 = KeyedContainer<arraykey, mixed>|Container<mixed>;
              type AliasCT220 = CT220;

  
class CheckAliasCT220<T as AliasCT220> extends BaseCheck {
  const type T = AliasCT220;
  const string NAME = 'AliasCT220';

  <<__LateInit>>
  private AliasCT220 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT220 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT220 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT220>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT220>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT220 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT220> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT221 = KeyedTraversable<arraykey, mixed>|Container<mixed>;
              type AliasCT221 = CT221;

  
class CheckAliasCT221<T as AliasCT221> extends BaseCheck {
  const type T = AliasCT221;
  const string NAME = 'AliasCT221';

  <<__LateInit>>
  private AliasCT221 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT221 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT221 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT221>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT221>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT221 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT221> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT222 = MyTrait|Container<mixed>;
              type AliasCT222 = CT222;

  
class CheckAliasCT222<T as AliasCT222> extends BaseCheck {
  const type T = AliasCT222;
  const string NAME = 'AliasCT222';

  <<__LateInit>>
  private AliasCT222 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT222 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT222 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT222>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT222>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT222 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT222> {
    return vec[keyset[],vec[]];
  }
}
case type CT223 = ReifiedClass<null>|Container<mixed>;
              type AliasCT223 = CT223;

  
class CheckAliasCT223<T as AliasCT223> extends BaseCheck {
  const type T = AliasCT223;
  const string NAME = 'AliasCT223';

  <<__LateInit>>
  private AliasCT223 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT223 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT223 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT223>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT223>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT223 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT223> {
    return vec[keyset[],new ReifiedClass<null>(),vec[]];
  }
}
case type CT224 = Stringish|Container<mixed>;
              type AliasCT224 = CT224;

  
class CheckAliasCT224<T as AliasCT224> extends BaseCheck {
  const type T = AliasCT224;
  const string NAME = 'AliasCT224';

  <<__LateInit>>
  private AliasCT224 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT224 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT224 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT224>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT224>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT224 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT224> {
    return vec['','hello world',keyset[],new StringishObj(),vec[]];
  }
}
case type CT225 = Traversable<mixed>|Container<mixed>;
              type AliasCT225 = CT225;

  
class CheckAliasCT225<T as AliasCT225> extends BaseCheck {
  const type T = AliasCT225;
  const string NAME = 'AliasCT225';

  <<__LateInit>>
  private AliasCT225 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT225 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT225 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT225>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT225>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT225 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT225> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT226 = XHPChild|Container<mixed>;
              type AliasCT226 = CT226;

  
class CheckAliasCT226<T as AliasCT226> extends BaseCheck {
  const type T = AliasCT226;
  const string NAME = 'AliasCT226';

  <<__LateInit>>
  private AliasCT226 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT226 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT226 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT226>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT226>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT226 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT226> {
    return vec['','hello world',0,1,<my-xhp/>,keyset[],vec[]];
  }
}
case type CT227 = arraykey|Container<mixed>;
              type AliasCT227 = CT227;

  
class CheckAliasCT227<T as AliasCT227> extends BaseCheck {
  const type T = AliasCT227;
  const string NAME = 'AliasCT227';

  <<__LateInit>>
  private AliasCT227 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT227 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT227 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT227>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT227>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT227 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT227> {
    return vec['','hello world',0,1,keyset[],vec[]];
  }
}
case type CT228 = bool|Container<mixed>;
              type AliasCT228 = CT228;

  
class CheckAliasCT228<T as AliasCT228> extends BaseCheck {
  const type T = AliasCT228;
  const string NAME = 'AliasCT228';

  <<__LateInit>>
  private AliasCT228 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT228 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT228 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT228>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT228>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT228 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT228> {
    return vec[false,keyset[],true,vec[]];
  }
}
case type CT229 = dict<arraykey, mixed>|Container<mixed>;
              type AliasCT229 = CT229;

  
class CheckAliasCT229<T as AliasCT229> extends BaseCheck {
  const type T = AliasCT229;
  const string NAME = 'AliasCT229';

  <<__LateInit>>
  private AliasCT229 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT229 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT229 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT229>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT229>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT229 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT229> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT230 = dynamic|Container<mixed>;
              type AliasCT230 = CT230;

  
class CheckAliasCT230<T as AliasCT230> extends BaseCheck {
  const type T = AliasCT230;
  const string NAME = 'AliasCT230';

  <<__LateInit>>
  private AliasCT230 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT230 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT230 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT230>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT230>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT230 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT230> {
    return vec[false,keyset[],null,shape('x' => 10),shape(),true,vec[]];
  }
}
case type CT231 = float|Container<mixed>;
              type AliasCT231 = CT231;

  
class CheckAliasCT231<T as AliasCT231> extends BaseCheck {
  const type T = AliasCT231;
  const string NAME = 'AliasCT231';

  <<__LateInit>>
  private AliasCT231 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT231 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT231 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT231>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT231>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT231 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT231> {
    return vec[0.0,3.14,keyset[],vec[]];
  }
}
case type CT232 = int|Container<mixed>;
              type AliasCT232 = CT232;

  
class CheckAliasCT232<T as AliasCT232> extends BaseCheck {
  const type T = AliasCT232;
  const string NAME = 'AliasCT232';

  <<__LateInit>>
  private AliasCT232 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT232 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT232 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT232>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT232>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT232 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT232> {
    return vec[0,1,keyset[],vec[]];
  }
}
case type CT233 = keyset<arraykey>|Container<mixed>;
              type AliasCT233 = CT233;

  
class CheckAliasCT233<T as AliasCT233> extends BaseCheck {
  const type T = AliasCT233;
  const string NAME = 'AliasCT233';

  <<__LateInit>>
  private AliasCT233 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT233 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT233 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT233>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT233>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT233 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT233> {
    return vec[keyset[],vec[]];
  }
}
case type CT234 = mixed|Container<mixed>;
              type AliasCT234 = CT234;

  
class CheckAliasCT234<T as AliasCT234> extends BaseCheck {
  const type T = AliasCT234;
  const string NAME = 'AliasCT234';

  <<__LateInit>>
  private AliasCT234 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT234 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT234 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT234>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT234>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT234 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT234> {
    return vec['','hello world',0,1,false,keyset[],null,true,vec[]];
  }
}
case type CT235 = nonnull|Container<mixed>;
              type AliasCT235 = CT235;

  
class CheckAliasCT235<T as AliasCT235> extends BaseCheck {
  const type T = AliasCT235;
  const string NAME = 'AliasCT235';

  <<__LateInit>>
  private AliasCT235 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT235 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT235 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT235>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT235>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT235 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT235> {
    return vec['','hello world',0,1,false,keyset[],true,vec[]];
  }
}
case type CT236 = noreturn|Container<mixed>;
              type AliasCT236 = CT236;

  
class CheckAliasCT236<T as AliasCT236> extends BaseCheck {
  const type T = AliasCT236;
  const string NAME = 'AliasCT236';

  <<__LateInit>>
  private AliasCT236 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT236 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT236 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT236>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT236>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT236 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT236> {
    return vec[keyset[],vec[]];
  }
}
case type CT237 = nothing|Container<mixed>;
              type AliasCT237 = CT237;

  
class CheckAliasCT237<T as AliasCT237> extends BaseCheck {
  const type T = AliasCT237;
  const string NAME = 'AliasCT237';

  <<__LateInit>>
  private AliasCT237 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT237 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT237 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT237>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT237>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT237 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT237> {
    return vec[keyset[],vec[]];
  }
}
case type CT238 = null|Container<mixed>;
              type AliasCT238 = CT238;

  
class CheckAliasCT238<T as AliasCT238> extends BaseCheck {
  const type T = AliasCT238;
  const string NAME = 'AliasCT238';

  <<__LateInit>>
  private AliasCT238 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT238 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT238 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT238>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT238>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT238 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT238> {
    return vec[keyset[],null,vec[]];
  }
}
case type CT239 = num|Container<mixed>;
              type AliasCT239 = CT239;

  
class CheckAliasCT239<T as AliasCT239> extends BaseCheck {
  const type T = AliasCT239;
  const string NAME = 'AliasCT239';

  <<__LateInit>>
  private AliasCT239 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT239 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT239 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT239>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT239>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT239 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT239> {
    return vec[0,0.0,1,3.14,keyset[],vec[]];
  }
}
case type CT240 = resource|Container<mixed>;
              type AliasCT240 = CT240;

  
class CheckAliasCT240<T as AliasCT240> extends BaseCheck {
  const type T = AliasCT240;
  const string NAME = 'AliasCT240';

  <<__LateInit>>
  private AliasCT240 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT240 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT240 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT240>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT240>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT240 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT240> {
    return vec[imagecreate(10, 10),keyset[],vec[]];
  }
}
case type CT241 = shape(...)|Container<mixed>;
              type AliasCT241 = CT241;

  
class CheckAliasCT241<T as AliasCT241> extends BaseCheck {
  const type T = AliasCT241;
  const string NAME = 'AliasCT241';

  <<__LateInit>>
  private AliasCT241 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT241 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT241 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT241>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT241>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT241 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT241> {
    return vec[keyset[],shape('x' => 10),shape(),vec[]];
  }
}
case type CT242 = string|Container<mixed>;
              type AliasCT242 = CT242;

  
class CheckAliasCT242<T as AliasCT242> extends BaseCheck {
  const type T = AliasCT242;
  const string NAME = 'AliasCT242';

  <<__LateInit>>
  private AliasCT242 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT242 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT242 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT242>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT242>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT242 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT242> {
    return vec['','hello world',keyset[],vec[]];
  }
}
case type CT243 = vec<mixed>|Container<mixed>;
              type AliasCT243 = CT243;

  
class CheckAliasCT243<T as AliasCT243> extends BaseCheck {
  const type T = AliasCT243;
  const string NAME = 'AliasCT243';

  <<__LateInit>>
  private AliasCT243 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT243 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT243 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT243>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT243>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT243 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT243> {
    return vec[keyset[],vec[]];
  }
}
case type CT244 = vec_or_dict<string>|Container<mixed>;
              type AliasCT244 = CT244;

  
class CheckAliasCT244<T as AliasCT244> extends BaseCheck {
  const type T = AliasCT244;
  const string NAME = 'AliasCT244';

  <<__LateInit>>
  private AliasCT244 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT244 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT244 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT244>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT244>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT244 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT244> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT245 = void|Container<mixed>;
              type AliasCT245 = CT245;

  
class CheckAliasCT245<T as AliasCT245> extends BaseCheck {
  const type T = AliasCT245;
  const string NAME = 'AliasCT245';

  <<__LateInit>>
  private AliasCT245 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT245 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT245 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT245>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT245>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT245 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT245> {
    return vec[keyset[],null,vec[]];
  }
}
case type CT246 = HH\AnyArray<arraykey, mixed>|HH\AnyArray<arraykey, mixed>;
              type AliasCT246 = CT246;

  
class CheckAliasCT246<T as AliasCT246> extends BaseCheck {
  const type T = AliasCT246;
  const string NAME = 'AliasCT246';

  <<__LateInit>>
  private AliasCT246 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT246 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT246 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT246>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT246>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT246 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT246> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT247 = HH\EnumClass\Label<EC, float>|HH\AnyArray<arraykey, mixed>;
              type AliasCT247 = CT247;

  
class CheckAliasCT247<T as AliasCT247> extends BaseCheck {
  const type T = AliasCT247;
  const string NAME = 'AliasCT247';

  <<__LateInit>>
  private AliasCT247 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT247 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT247 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT247>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT247>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT247 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT247> {
    return vec[#A,EC#B,dict[],keyset[],vec[]];
  }
}
case type CT248 = HH\FunctionRef<(function(): void)>|HH\AnyArray<arraykey, mixed>;
              type AliasCT248 = CT248;

  
class CheckAliasCT248<T as AliasCT248> extends BaseCheck {
  const type T = AliasCT248;
  const string NAME = 'AliasCT248';

  <<__LateInit>>
  private AliasCT248 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT248 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT248 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT248>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT248>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT248 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT248> {
    return vec[dict[],keyset[],my_func<>,vec[]];
  }
}
case type CT249 = HH\MemberOf<EC, float>|HH\AnyArray<arraykey, mixed>;
              type AliasCT249 = CT249;

  
class CheckAliasCT249<T as AliasCT249> extends BaseCheck {
  const type T = AliasCT249;
  const string NAME = 'AliasCT249';

  <<__LateInit>>
  private AliasCT249 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT249 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT249 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT249>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT249>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT249 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT249> {
    return vec[EC::A,EC::B,dict[],keyset[],vec[]];
  }
}
case type CT250 = I|HH\AnyArray<arraykey, mixed>;
              type AliasCT250 = CT250;

  
class CheckAliasCT250<T as AliasCT250> extends BaseCheck {
  const type T = AliasCT250;
  const string NAME = 'AliasCT250';

  <<__LateInit>>
  private AliasCT250 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT250 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT250 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT250>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT250>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT250 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT250> {
    return vec[dict[],keyset[],new InstanceOfI(),vec[]];
  }
}
case type CT251 = KeyedContainer<arraykey, mixed>|HH\AnyArray<arraykey, mixed>;
              type AliasCT251 = CT251;

  
class CheckAliasCT251<T as AliasCT251> extends BaseCheck {
  const type T = AliasCT251;
  const string NAME = 'AliasCT251';

  <<__LateInit>>
  private AliasCT251 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT251 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT251 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT251>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT251>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT251 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT251> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT252 = KeyedTraversable<arraykey, mixed>|HH\AnyArray<arraykey, mixed>;
              type AliasCT252 = CT252;

  
class CheckAliasCT252<T as AliasCT252> extends BaseCheck {
  const type T = AliasCT252;
  const string NAME = 'AliasCT252';

  <<__LateInit>>
  private AliasCT252 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT252 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT252 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT252>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT252>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT252 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT252> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT253 = MyTrait|HH\AnyArray<arraykey, mixed>;
              type AliasCT253 = CT253;

  
class CheckAliasCT253<T as AliasCT253> extends BaseCheck {
  const type T = AliasCT253;
  const string NAME = 'AliasCT253';

  <<__LateInit>>
  private AliasCT253 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT253 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT253 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT253>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT253>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT253 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT253> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT254 = ReifiedClass<null>|HH\AnyArray<arraykey, mixed>;
              type AliasCT254 = CT254;

  
class CheckAliasCT254<T as AliasCT254> extends BaseCheck {
  const type T = AliasCT254;
  const string NAME = 'AliasCT254';

  <<__LateInit>>
  private AliasCT254 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT254 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT254 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT254>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT254>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT254 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT254> {
    return vec[dict[],keyset[],new ReifiedClass<null>(),vec[]];
  }
}
case type CT255 = Stringish|HH\AnyArray<arraykey, mixed>;
              type AliasCT255 = CT255;

  
class CheckAliasCT255<T as AliasCT255> extends BaseCheck {
  const type T = AliasCT255;
  const string NAME = 'AliasCT255';

  <<__LateInit>>
  private AliasCT255 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT255 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT255 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT255>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT255>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT255 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT255> {
    return vec['','hello world',dict[],keyset[],new StringishObj(),vec[]];
  }
}
case type CT256 = Traversable<mixed>|HH\AnyArray<arraykey, mixed>;
              type AliasCT256 = CT256;

  
class CheckAliasCT256<T as AliasCT256> extends BaseCheck {
  const type T = AliasCT256;
  const string NAME = 'AliasCT256';

  <<__LateInit>>
  private AliasCT256 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT256 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT256 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT256>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT256>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT256 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT256> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT257 = XHPChild|HH\AnyArray<arraykey, mixed>;
              type AliasCT257 = CT257;

  
class CheckAliasCT257<T as AliasCT257> extends BaseCheck {
  const type T = AliasCT257;
  const string NAME = 'AliasCT257';

  <<__LateInit>>
  private AliasCT257 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT257 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT257 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT257>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT257>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT257 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT257> {
    return vec['','hello world',0,1,<my-xhp/>,dict[],keyset[],vec[]];
  }
}
case type CT258 = arraykey|HH\AnyArray<arraykey, mixed>;
              type AliasCT258 = CT258;

  
class CheckAliasCT258<T as AliasCT258> extends BaseCheck {
  const type T = AliasCT258;
  const string NAME = 'AliasCT258';

  <<__LateInit>>
  private AliasCT258 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT258 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT258 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT258>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT258>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT258 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT258> {
    return vec['','hello world',0,1,dict[],keyset[],vec[]];
  }
}
case type CT259 = bool|HH\AnyArray<arraykey, mixed>;
              type AliasCT259 = CT259;

  
class CheckAliasCT259<T as AliasCT259> extends BaseCheck {
  const type T = AliasCT259;
  const string NAME = 'AliasCT259';

  <<__LateInit>>
  private AliasCT259 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT259 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT259 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT259>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT259>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT259 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT259> {
    return vec[dict[],false,keyset[],true,vec[]];
  }
}
case type CT260 = dict<arraykey, mixed>|HH\AnyArray<arraykey, mixed>;
              type AliasCT260 = CT260;

  
class CheckAliasCT260<T as AliasCT260> extends BaseCheck {
  const type T = AliasCT260;
  const string NAME = 'AliasCT260';

  <<__LateInit>>
  private AliasCT260 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT260 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT260 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT260>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT260>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT260 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT260> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT261 = dynamic|HH\AnyArray<arraykey, mixed>;
              type AliasCT261 = CT261;

  
class CheckAliasCT261<T as AliasCT261> extends BaseCheck {
  const type T = AliasCT261;
  const string NAME = 'AliasCT261';

  <<__LateInit>>
  private AliasCT261 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT261 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT261 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT261>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT261>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT261 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT261> {
    return vec[dict[],false,keyset[],null,shape('x' => 10),shape(),true,vec[]];
  }
}
case type CT262 = float|HH\AnyArray<arraykey, mixed>;
              type AliasCT262 = CT262;

  
class CheckAliasCT262<T as AliasCT262> extends BaseCheck {
  const type T = AliasCT262;
  const string NAME = 'AliasCT262';

  <<__LateInit>>
  private AliasCT262 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT262 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT262 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT262>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT262>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT262 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT262> {
    return vec[0.0,3.14,dict[],keyset[],vec[]];
  }
}
case type CT263 = int|HH\AnyArray<arraykey, mixed>;
              type AliasCT263 = CT263;

  
class CheckAliasCT263<T as AliasCT263> extends BaseCheck {
  const type T = AliasCT263;
  const string NAME = 'AliasCT263';

  <<__LateInit>>
  private AliasCT263 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT263 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT263 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT263>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT263>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT263 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT263> {
    return vec[0,1,dict[],keyset[],vec[]];
  }
}
case type CT264 = keyset<arraykey>|HH\AnyArray<arraykey, mixed>;
              type AliasCT264 = CT264;

  
class CheckAliasCT264<T as AliasCT264> extends BaseCheck {
  const type T = AliasCT264;
  const string NAME = 'AliasCT264';

  <<__LateInit>>
  private AliasCT264 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT264 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT264 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT264>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT264>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT264 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT264> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT265 = mixed|HH\AnyArray<arraykey, mixed>;
              type AliasCT265 = CT265;

  
class CheckAliasCT265<T as AliasCT265> extends BaseCheck {
  const type T = AliasCT265;
  const string NAME = 'AliasCT265';

  <<__LateInit>>
  private AliasCT265 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT265 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT265 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT265>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT265>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT265 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT265> {
    return vec['','hello world',0,1,dict[],false,keyset[],null,true,vec[]];
  }
}
case type CT266 = nonnull|HH\AnyArray<arraykey, mixed>;
              type AliasCT266 = CT266;

  
class CheckAliasCT266<T as AliasCT266> extends BaseCheck {
  const type T = AliasCT266;
  const string NAME = 'AliasCT266';

  <<__LateInit>>
  private AliasCT266 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT266 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT266 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT266>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT266>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT266 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT266> {
    return vec['','hello world',0,1,dict[],false,keyset[],true,vec[]];
  }
}
case type CT267 = noreturn|HH\AnyArray<arraykey, mixed>;
              type AliasCT267 = CT267;

  
class CheckAliasCT267<T as AliasCT267> extends BaseCheck {
  const type T = AliasCT267;
  const string NAME = 'AliasCT267';

  <<__LateInit>>
  private AliasCT267 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT267 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT267 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT267>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT267>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT267 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT267> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT268 = nothing|HH\AnyArray<arraykey, mixed>;
              type AliasCT268 = CT268;

  
class CheckAliasCT268<T as AliasCT268> extends BaseCheck {
  const type T = AliasCT268;
  const string NAME = 'AliasCT268';

  <<__LateInit>>
  private AliasCT268 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT268 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT268 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT268>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT268>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT268 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT268> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT269 = null|HH\AnyArray<arraykey, mixed>;
              type AliasCT269 = CT269;

  
class CheckAliasCT269<T as AliasCT269> extends BaseCheck {
  const type T = AliasCT269;
  const string NAME = 'AliasCT269';

  <<__LateInit>>
  private AliasCT269 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT269 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT269 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT269>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT269>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT269 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT269> {
    return vec[dict[],keyset[],null,vec[]];
  }
}
case type CT270 = num|HH\AnyArray<arraykey, mixed>;
              type AliasCT270 = CT270;

  
class CheckAliasCT270<T as AliasCT270> extends BaseCheck {
  const type T = AliasCT270;
  const string NAME = 'AliasCT270';

  <<__LateInit>>
  private AliasCT270 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT270 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT270 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT270>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT270>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT270 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT270> {
    return vec[0,0.0,1,3.14,dict[],keyset[],vec[]];
  }
}
case type CT271 = resource|HH\AnyArray<arraykey, mixed>;
              type AliasCT271 = CT271;

  
class CheckAliasCT271<T as AliasCT271> extends BaseCheck {
  const type T = AliasCT271;
  const string NAME = 'AliasCT271';

  <<__LateInit>>
  private AliasCT271 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT271 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT271 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT271>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT271>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT271 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT271> {
    return vec[dict[],imagecreate(10, 10),keyset[],vec[]];
  }
}
case type CT272 = shape(...)|HH\AnyArray<arraykey, mixed>;
              type AliasCT272 = CT272;

  
class CheckAliasCT272<T as AliasCT272> extends BaseCheck {
  const type T = AliasCT272;
  const string NAME = 'AliasCT272';

  <<__LateInit>>
  private AliasCT272 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT272 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT272 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT272>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT272>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT272 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT272> {
    return vec[dict[],keyset[],shape('x' => 10),shape(),vec[]];
  }
}
case type CT273 = string|HH\AnyArray<arraykey, mixed>;
              type AliasCT273 = CT273;

  
class CheckAliasCT273<T as AliasCT273> extends BaseCheck {
  const type T = AliasCT273;
  const string NAME = 'AliasCT273';

  <<__LateInit>>
  private AliasCT273 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT273 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT273 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT273>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT273>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT273 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT273> {
    return vec['','hello world',dict[],keyset[],vec[]];
  }
}
case type CT274 = vec<mixed>|HH\AnyArray<arraykey, mixed>;
              type AliasCT274 = CT274;

  
class CheckAliasCT274<T as AliasCT274> extends BaseCheck {
  const type T = AliasCT274;
  const string NAME = 'AliasCT274';

  <<__LateInit>>
  private AliasCT274 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT274 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT274 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT274>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT274>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT274 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT274> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT275 = vec_or_dict<string>|HH\AnyArray<arraykey, mixed>;
              type AliasCT275 = CT275;

  
class CheckAliasCT275<T as AliasCT275> extends BaseCheck {
  const type T = AliasCT275;
  const string NAME = 'AliasCT275';

  <<__LateInit>>
  private AliasCT275 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT275 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT275 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT275>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT275>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT275 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT275> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT276 = void|HH\AnyArray<arraykey, mixed>;
              type AliasCT276 = CT276;

  
class CheckAliasCT276<T as AliasCT276> extends BaseCheck {
  const type T = AliasCT276;
  const string NAME = 'AliasCT276';

  <<__LateInit>>
  private AliasCT276 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT276 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT276 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT276>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT276>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT276 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT276> {
    return vec[dict[],keyset[],null,vec[]];
  }
}
case type CT277 = HH\EnumClass\Label<EC, float>|HH\EnumClass\Label<EC, float>;
              type AliasCT277 = CT277;

  
class CheckAliasCT277<T as AliasCT277> extends BaseCheck {
  const type T = AliasCT277;
  const string NAME = 'AliasCT277';

  <<__LateInit>>
  private AliasCT277 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT277 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT277 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT277>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT277>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT277 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT277> {
    return vec[#A,EC#B];
  }
}
case type CT278 = HH\FunctionRef<(function(): void)>|HH\EnumClass\Label<EC, float>;
              type AliasCT278 = CT278;

  
class CheckAliasCT278<T as AliasCT278> extends BaseCheck {
  const type T = AliasCT278;
  const string NAME = 'AliasCT278';

  <<__LateInit>>
  private AliasCT278 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT278 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT278 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT278>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT278>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT278 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT278> {
    return vec[#A,EC#B,my_func<>];
  }
}
case type CT279 = HH\MemberOf<EC, float>|HH\EnumClass\Label<EC, float>;
              type AliasCT279 = CT279;

  
class CheckAliasCT279<T as AliasCT279> extends BaseCheck {
  const type T = AliasCT279;
  const string NAME = 'AliasCT279';

  <<__LateInit>>
  private AliasCT279 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT279 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT279 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT279>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT279>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT279 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT279> {
    return vec[#A,EC#B,EC::A,EC::B];
  }
}
case type CT280 = I|HH\EnumClass\Label<EC, float>;
              type AliasCT280 = CT280;

  
class CheckAliasCT280<T as AliasCT280> extends BaseCheck {
  const type T = AliasCT280;
  const string NAME = 'AliasCT280';

  <<__LateInit>>
  private AliasCT280 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT280 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT280 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT280>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT280>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT280 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT280> {
    return vec[#A,EC#B,new InstanceOfI()];
  }
}
case type CT281 = KeyedContainer<arraykey, mixed>|HH\EnumClass\Label<EC, float>;
              type AliasCT281 = CT281;

  
class CheckAliasCT281<T as AliasCT281> extends BaseCheck {
  const type T = AliasCT281;
  const string NAME = 'AliasCT281';

  <<__LateInit>>
  private AliasCT281 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT281 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT281 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT281>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT281>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT281 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT281> {
    return vec[#A,EC#B,dict[],vec[]];
  }
}
case type CT282 = KeyedTraversable<arraykey, mixed>|HH\EnumClass\Label<EC, float>;
              type AliasCT282 = CT282;

  
class CheckAliasCT282<T as AliasCT282> extends BaseCheck {
  const type T = AliasCT282;
  const string NAME = 'AliasCT282';

  <<__LateInit>>
  private AliasCT282 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT282 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT282 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT282>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT282>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT282 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT282> {
    return vec[#A,EC#B,dict[],keyset[],vec[]];
  }
}
case type CT283 = MyTrait|HH\EnumClass\Label<EC, float>;
              type AliasCT283 = CT283;

  
class CheckAliasCT283<T as AliasCT283> extends BaseCheck {
  const type T = AliasCT283;
  const string NAME = 'AliasCT283';

  <<__LateInit>>
  private AliasCT283 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT283 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT283 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT283>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT283>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT283 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT283> {
    return vec[#A,EC#B];
  }
}
case type CT284 = ReifiedClass<null>|HH\EnumClass\Label<EC, float>;
              type AliasCT284 = CT284;

  
class CheckAliasCT284<T as AliasCT284> extends BaseCheck {
  const type T = AliasCT284;
  const string NAME = 'AliasCT284';

  <<__LateInit>>
  private AliasCT284 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT284 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT284 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT284>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT284>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT284 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT284> {
    return vec[#A,EC#B,new ReifiedClass<null>()];
  }
}
case type CT285 = Stringish|HH\EnumClass\Label<EC, float>;
              type AliasCT285 = CT285;

  
class CheckAliasCT285<T as AliasCT285> extends BaseCheck {
  const type T = AliasCT285;
  const string NAME = 'AliasCT285';

  <<__LateInit>>
  private AliasCT285 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT285 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT285 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT285>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT285>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT285 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT285> {
    return vec[#A,'','hello world',EC#B,new StringishObj()];
  }
}
case type CT286 = Traversable<mixed>|HH\EnumClass\Label<EC, float>;
              type AliasCT286 = CT286;

  
class CheckAliasCT286<T as AliasCT286> extends BaseCheck {
  const type T = AliasCT286;
  const string NAME = 'AliasCT286';

  <<__LateInit>>
  private AliasCT286 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT286 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT286 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT286>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT286>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT286 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT286> {
    return vec[#A,EC#B,dict[],keyset[],vec[]];
  }
}
case type CT287 = XHPChild|HH\EnumClass\Label<EC, float>;
              type AliasCT287 = CT287;

  
class CheckAliasCT287<T as AliasCT287> extends BaseCheck {
  const type T = AliasCT287;
  const string NAME = 'AliasCT287';

  <<__LateInit>>
  private AliasCT287 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT287 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT287 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT287>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT287>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT287 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT287> {
    return vec[#A,'','hello world',0,1,<my-xhp/>,EC#B];
  }
}
case type CT288 = arraykey|HH\EnumClass\Label<EC, float>;
              type AliasCT288 = CT288;

  
class CheckAliasCT288<T as AliasCT288> extends BaseCheck {
  const type T = AliasCT288;
  const string NAME = 'AliasCT288';

  <<__LateInit>>
  private AliasCT288 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT288 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT288 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT288>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT288>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT288 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT288> {
    return vec[#A,'','hello world',0,1,EC#B];
  }
}
case type CT289 = bool|HH\EnumClass\Label<EC, float>;
              type AliasCT289 = CT289;

  
class CheckAliasCT289<T as AliasCT289> extends BaseCheck {
  const type T = AliasCT289;
  const string NAME = 'AliasCT289';

  <<__LateInit>>
  private AliasCT289 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT289 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT289 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT289>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT289>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT289 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT289> {
    return vec[#A,EC#B,false,true];
  }
}
case type CT290 = dict<arraykey, mixed>|HH\EnumClass\Label<EC, float>;
              type AliasCT290 = CT290;

  
class CheckAliasCT290<T as AliasCT290> extends BaseCheck {
  const type T = AliasCT290;
  const string NAME = 'AliasCT290';

  <<__LateInit>>
  private AliasCT290 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT290 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT290 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT290>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT290>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT290 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT290> {
    return vec[#A,EC#B,dict[]];
  }
}
case type CT291 = dynamic|HH\EnumClass\Label<EC, float>;
              type AliasCT291 = CT291;

  
class CheckAliasCT291<T as AliasCT291> extends BaseCheck {
  const type T = AliasCT291;
  const string NAME = 'AliasCT291';

  <<__LateInit>>
  private AliasCT291 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT291 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT291 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT291>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT291>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT291 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT291> {
    return vec[#A,EC#B,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT292 = float|HH\EnumClass\Label<EC, float>;
              type AliasCT292 = CT292;

  
class CheckAliasCT292<T as AliasCT292> extends BaseCheck {
  const type T = AliasCT292;
  const string NAME = 'AliasCT292';

  <<__LateInit>>
  private AliasCT292 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT292 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT292 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT292>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT292>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT292 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT292> {
    return vec[#A,0.0,3.14,EC#B];
  }
}
case type CT293 = int|HH\EnumClass\Label<EC, float>;
              type AliasCT293 = CT293;

  
class CheckAliasCT293<T as AliasCT293> extends BaseCheck {
  const type T = AliasCT293;
  const string NAME = 'AliasCT293';

  <<__LateInit>>
  private AliasCT293 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT293 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT293 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT293>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT293>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT293 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT293> {
    return vec[#A,0,1,EC#B];
  }
}
case type CT294 = keyset<arraykey>|HH\EnumClass\Label<EC, float>;
              type AliasCT294 = CT294;

  
class CheckAliasCT294<T as AliasCT294> extends BaseCheck {
  const type T = AliasCT294;
  const string NAME = 'AliasCT294';

  <<__LateInit>>
  private AliasCT294 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT294 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT294 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT294>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT294>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT294 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT294> {
    return vec[#A,EC#B,keyset[]];
  }
}
case type CT295 = mixed|HH\EnumClass\Label<EC, float>;
              type AliasCT295 = CT295;

  
class CheckAliasCT295<T as AliasCT295> extends BaseCheck {
  const type T = AliasCT295;
  const string NAME = 'AliasCT295';

  <<__LateInit>>
  private AliasCT295 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT295 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT295 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT295>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT295>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT295 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT295> {
    return vec[#A,'','hello world',0,1,EC#B,false,null,true];
  }
}
case type CT296 = nonnull|HH\EnumClass\Label<EC, float>;
              type AliasCT296 = CT296;

  
class CheckAliasCT296<T as AliasCT296> extends BaseCheck {
  const type T = AliasCT296;
  const string NAME = 'AliasCT296';

  <<__LateInit>>
  private AliasCT296 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT296 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT296 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT296>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT296>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT296 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT296> {
    return vec[#A,'','hello world',0,1,EC#B,false,true];
  }
}
case type CT297 = noreturn|HH\EnumClass\Label<EC, float>;
              type AliasCT297 = CT297;

  
class CheckAliasCT297<T as AliasCT297> extends BaseCheck {
  const type T = AliasCT297;
  const string NAME = 'AliasCT297';

  <<__LateInit>>
  private AliasCT297 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT297 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT297 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT297>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT297>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT297 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT297> {
    return vec[#A,EC#B];
  }
}
case type CT298 = nothing|HH\EnumClass\Label<EC, float>;
              type AliasCT298 = CT298;

  
class CheckAliasCT298<T as AliasCT298> extends BaseCheck {
  const type T = AliasCT298;
  const string NAME = 'AliasCT298';

  <<__LateInit>>
  private AliasCT298 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT298 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT298 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT298>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT298>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT298 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT298> {
    return vec[#A,EC#B];
  }
}
case type CT299 = null|HH\EnumClass\Label<EC, float>;
              type AliasCT299 = CT299;

  
class CheckAliasCT299<T as AliasCT299> extends BaseCheck {
  const type T = AliasCT299;
  const string NAME = 'AliasCT299';

  <<__LateInit>>
  private AliasCT299 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT299 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT299 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT299>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT299>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT299 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT299> {
    return vec[#A,EC#B,null];
  }
}
case type CT300 = num|HH\EnumClass\Label<EC, float>;
              type AliasCT300 = CT300;

  
class CheckAliasCT300<T as AliasCT300> extends BaseCheck {
  const type T = AliasCT300;
  const string NAME = 'AliasCT300';

  <<__LateInit>>
  private AliasCT300 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT300 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT300 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT300>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT300>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT300 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT300> {
    return vec[#A,0,0.0,1,3.14,EC#B];
  }
}
case type CT301 = resource|HH\EnumClass\Label<EC, float>;
              type AliasCT301 = CT301;

  
class CheckAliasCT301<T as AliasCT301> extends BaseCheck {
  const type T = AliasCT301;
  const string NAME = 'AliasCT301';

  <<__LateInit>>
  private AliasCT301 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT301 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT301 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT301>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT301>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT301 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT301> {
    return vec[#A,EC#B,imagecreate(10, 10)];
  }
}
case type CT302 = shape(...)|HH\EnumClass\Label<EC, float>;
              type AliasCT302 = CT302;

  
class CheckAliasCT302<T as AliasCT302> extends BaseCheck {
  const type T = AliasCT302;
  const string NAME = 'AliasCT302';

  <<__LateInit>>
  private AliasCT302 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT302 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT302 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT302>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT302>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT302 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT302> {
    return vec[#A,EC#B,shape('x' => 10),shape()];
  }
}
case type CT303 = string|HH\EnumClass\Label<EC, float>;
              type AliasCT303 = CT303;

  
class CheckAliasCT303<T as AliasCT303> extends BaseCheck {
  const type T = AliasCT303;
  const string NAME = 'AliasCT303';

  <<__LateInit>>
  private AliasCT303 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT303 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT303 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT303>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT303>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT303 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT303> {
    return vec[#A,'','hello world',EC#B];
  }
}
case type CT304 = vec<mixed>|HH\EnumClass\Label<EC, float>;
              type AliasCT304 = CT304;

  
class CheckAliasCT304<T as AliasCT304> extends BaseCheck {
  const type T = AliasCT304;
  const string NAME = 'AliasCT304';

  <<__LateInit>>
  private AliasCT304 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT304 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT304 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT304>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT304>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT304 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT304> {
    return vec[#A,EC#B,vec[]];
  }
}
case type CT305 = vec_or_dict<string>|HH\EnumClass\Label<EC, float>;
              type AliasCT305 = CT305;

  
class CheckAliasCT305<T as AliasCT305> extends BaseCheck {
  const type T = AliasCT305;
  const string NAME = 'AliasCT305';

  <<__LateInit>>
  private AliasCT305 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT305 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT305 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT305>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT305>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT305 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT305> {
    return vec[#A,EC#B,dict[],vec[]];
  }
}
case type CT306 = void|HH\EnumClass\Label<EC, float>;
              type AliasCT306 = CT306;

  
class CheckAliasCT306<T as AliasCT306> extends BaseCheck {
  const type T = AliasCT306;
  const string NAME = 'AliasCT306';

  <<__LateInit>>
  private AliasCT306 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT306 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT306 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT306>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT306>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT306 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT306> {
    return vec[#A,EC#B,null];
  }
}
case type CT307 = HH\FunctionRef<(function(): void)>|HH\FunctionRef<(function(): void)>;
              type AliasCT307 = CT307;

  
class CheckAliasCT307<T as AliasCT307> extends BaseCheck {
  const type T = AliasCT307;
  const string NAME = 'AliasCT307';

  <<__LateInit>>
  private AliasCT307 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT307 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT307 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT307>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT307>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT307 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT307> {
    return vec[my_func<>];
  }
}
case type CT308 = HH\MemberOf<EC, float>|HH\FunctionRef<(function(): void)>;
              type AliasCT308 = CT308;

  
class CheckAliasCT308<T as AliasCT308> extends BaseCheck {
  const type T = AliasCT308;
  const string NAME = 'AliasCT308';

  <<__LateInit>>
  private AliasCT308 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT308 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT308 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT308>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT308>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT308 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT308> {
    return vec[EC::A,EC::B,my_func<>];
  }
}
case type CT309 = I|HH\FunctionRef<(function(): void)>;
              type AliasCT309 = CT309;

  
class CheckAliasCT309<T as AliasCT309> extends BaseCheck {
  const type T = AliasCT309;
  const string NAME = 'AliasCT309';

  <<__LateInit>>
  private AliasCT309 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT309 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT309 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT309>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT309>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT309 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT309> {
    return vec[my_func<>,new InstanceOfI()];
  }
}
case type CT310 = KeyedContainer<arraykey, mixed>|HH\FunctionRef<(function(): void)>;
              type AliasCT310 = CT310;

  
class CheckAliasCT310<T as AliasCT310> extends BaseCheck {
  const type T = AliasCT310;
  const string NAME = 'AliasCT310';

  <<__LateInit>>
  private AliasCT310 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT310 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT310 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT310>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT310>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT310 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT310> {
    return vec[dict[],my_func<>,vec[]];
  }
}
case type CT311 = KeyedTraversable<arraykey, mixed>|HH\FunctionRef<(function(): void)>;
              type AliasCT311 = CT311;

  
class CheckAliasCT311<T as AliasCT311> extends BaseCheck {
  const type T = AliasCT311;
  const string NAME = 'AliasCT311';

  <<__LateInit>>
  private AliasCT311 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT311 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT311 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT311>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT311>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT311 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT311> {
    return vec[dict[],keyset[],my_func<>,vec[]];
  }
}
case type CT312 = MyEnum|HH\FunctionRef<(function(): void)>;
              type AliasCT312 = CT312;

  
class CheckAliasCT312<T as AliasCT312> extends BaseCheck {
  const type T = AliasCT312;
  const string NAME = 'AliasCT312';

  <<__LateInit>>
  private AliasCT312 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT312 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT312 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT312>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT312>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT312 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT312> {
    return vec['B',MyEnum::A,my_func<>];
  }
}
case type CT313 = MyTrait|HH\FunctionRef<(function(): void)>;
              type AliasCT313 = CT313;

  
class CheckAliasCT313<T as AliasCT313> extends BaseCheck {
  const type T = AliasCT313;
  const string NAME = 'AliasCT313';

  <<__LateInit>>
  private AliasCT313 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT313 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT313 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT313>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT313>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT313 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT313> {
    return vec[my_func<>];
  }
}
case type CT314 = ReifiedClass<null>|HH\FunctionRef<(function(): void)>;
              type AliasCT314 = CT314;

  
class CheckAliasCT314<T as AliasCT314> extends BaseCheck {
  const type T = AliasCT314;
  const string NAME = 'AliasCT314';

  <<__LateInit>>
  private AliasCT314 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT314 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT314 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT314>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT314>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT314 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT314> {
    return vec[my_func<>,new ReifiedClass<null>()];
  }
}
case type CT315 = Stringish|HH\FunctionRef<(function(): void)>;
              type AliasCT315 = CT315;

  
class CheckAliasCT315<T as AliasCT315> extends BaseCheck {
  const type T = AliasCT315;
  const string NAME = 'AliasCT315';

  <<__LateInit>>
  private AliasCT315 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT315 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT315 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT315>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT315>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT315 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT315> {
    return vec['','hello world',my_func<>,new StringishObj()];
  }
}
case type CT316 = Traversable<mixed>|HH\FunctionRef<(function(): void)>;
              type AliasCT316 = CT316;

  
class CheckAliasCT316<T as AliasCT316> extends BaseCheck {
  const type T = AliasCT316;
  const string NAME = 'AliasCT316';

  <<__LateInit>>
  private AliasCT316 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT316 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT316 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT316>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT316>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT316 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT316> {
    return vec[dict[],keyset[],my_func<>,vec[]];
  }
}
case type CT317 = XHPChild|HH\FunctionRef<(function(): void)>;
              type AliasCT317 = CT317;

  
class CheckAliasCT317<T as AliasCT317> extends BaseCheck {
  const type T = AliasCT317;
  const string NAME = 'AliasCT317';

  <<__LateInit>>
  private AliasCT317 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT317 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT317 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT317>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT317>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT317 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT317> {
    return vec['','hello world',0,1,<my-xhp/>,my_func<>];
  }
}
case type CT318 = arraykey|HH\FunctionRef<(function(): void)>;
              type AliasCT318 = CT318;

  
class CheckAliasCT318<T as AliasCT318> extends BaseCheck {
  const type T = AliasCT318;
  const string NAME = 'AliasCT318';

  <<__LateInit>>
  private AliasCT318 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT318 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT318 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT318>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT318>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT318 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT318> {
    return vec['','hello world',0,1,my_func<>];
  }
}
case type CT319 = bool|HH\FunctionRef<(function(): void)>;
              type AliasCT319 = CT319;

  
class CheckAliasCT319<T as AliasCT319> extends BaseCheck {
  const type T = AliasCT319;
  const string NAME = 'AliasCT319';

  <<__LateInit>>
  private AliasCT319 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT319 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT319 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT319>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT319>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT319 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT319> {
    return vec[false,my_func<>,true];
  }
}
case type CT320 = dict<arraykey, mixed>|HH\FunctionRef<(function(): void)>;
              type AliasCT320 = CT320;

  
class CheckAliasCT320<T as AliasCT320> extends BaseCheck {
  const type T = AliasCT320;
  const string NAME = 'AliasCT320';

  <<__LateInit>>
  private AliasCT320 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT320 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT320 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT320>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT320>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT320 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT320> {
    return vec[dict[],my_func<>];
  }
}
case type CT321 = dynamic|HH\FunctionRef<(function(): void)>;
              type AliasCT321 = CT321;

  
class CheckAliasCT321<T as AliasCT321> extends BaseCheck {
  const type T = AliasCT321;
  const string NAME = 'AliasCT321';

  <<__LateInit>>
  private AliasCT321 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT321 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT321 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT321>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT321>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT321 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT321> {
    return vec[false,my_func<>,null,shape('x' => 10),shape(),true];
  }
}
case type CT322 = float|HH\FunctionRef<(function(): void)>;
              type AliasCT322 = CT322;

  
class CheckAliasCT322<T as AliasCT322> extends BaseCheck {
  const type T = AliasCT322;
  const string NAME = 'AliasCT322';

  <<__LateInit>>
  private AliasCT322 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT322 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT322 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT322>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT322>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT322 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT322> {
    return vec[0.0,3.14,my_func<>];
  }
}
case type CT323 = int|HH\FunctionRef<(function(): void)>;
              type AliasCT323 = CT323;

  
class CheckAliasCT323<T as AliasCT323> extends BaseCheck {
  const type T = AliasCT323;
  const string NAME = 'AliasCT323';

  <<__LateInit>>
  private AliasCT323 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT323 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT323 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT323>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT323>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT323 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT323> {
    return vec[0,1,my_func<>];
  }
}
case type CT324 = keyset<arraykey>|HH\FunctionRef<(function(): void)>;
              type AliasCT324 = CT324;

  
class CheckAliasCT324<T as AliasCT324> extends BaseCheck {
  const type T = AliasCT324;
  const string NAME = 'AliasCT324';

  <<__LateInit>>
  private AliasCT324 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT324 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT324 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT324>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT324>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT324 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT324> {
    return vec[keyset[],my_func<>];
  }
}
case type CT325 = mixed|HH\FunctionRef<(function(): void)>;
              type AliasCT325 = CT325;

  
class CheckAliasCT325<T as AliasCT325> extends BaseCheck {
  const type T = AliasCT325;
  const string NAME = 'AliasCT325';

  <<__LateInit>>
  private AliasCT325 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT325 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT325 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT325>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT325>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT325 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT325> {
    return vec['','hello world',0,1,false,my_func<>,null,true];
  }
}
case type CT326 = nonnull|HH\FunctionRef<(function(): void)>;
              type AliasCT326 = CT326;

  
class CheckAliasCT326<T as AliasCT326> extends BaseCheck {
  const type T = AliasCT326;
  const string NAME = 'AliasCT326';

  <<__LateInit>>
  private AliasCT326 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT326 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT326 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT326>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT326>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT326 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT326> {
    return vec['','hello world',0,1,false,my_func<>,true];
  }
}
case type CT327 = noreturn|HH\FunctionRef<(function(): void)>;
              type AliasCT327 = CT327;

  
class CheckAliasCT327<T as AliasCT327> extends BaseCheck {
  const type T = AliasCT327;
  const string NAME = 'AliasCT327';

  <<__LateInit>>
  private AliasCT327 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT327 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT327 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT327>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT327>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT327 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT327> {
    return vec[my_func<>];
  }
}
case type CT328 = nothing|HH\FunctionRef<(function(): void)>;
              type AliasCT328 = CT328;

  
class CheckAliasCT328<T as AliasCT328> extends BaseCheck {
  const type T = AliasCT328;
  const string NAME = 'AliasCT328';

  <<__LateInit>>
  private AliasCT328 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT328 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT328 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT328>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT328>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT328 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT328> {
    return vec[my_func<>];
  }
}
case type CT329 = null|HH\FunctionRef<(function(): void)>;
              type AliasCT329 = CT329;

  
class CheckAliasCT329<T as AliasCT329> extends BaseCheck {
  const type T = AliasCT329;
  const string NAME = 'AliasCT329';

  <<__LateInit>>
  private AliasCT329 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT329 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT329 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT329>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT329>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT329 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT329> {
    return vec[my_func<>,null];
  }
}
case type CT330 = num|HH\FunctionRef<(function(): void)>;
              type AliasCT330 = CT330;

  
class CheckAliasCT330<T as AliasCT330> extends BaseCheck {
  const type T = AliasCT330;
  const string NAME = 'AliasCT330';

  <<__LateInit>>
  private AliasCT330 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT330 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT330 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT330>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT330>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT330 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT330> {
    return vec[0,0.0,1,3.14,my_func<>];
  }
}
case type CT331 = resource|HH\FunctionRef<(function(): void)>;
              type AliasCT331 = CT331;

  
class CheckAliasCT331<T as AliasCT331> extends BaseCheck {
  const type T = AliasCT331;
  const string NAME = 'AliasCT331';

  <<__LateInit>>
  private AliasCT331 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT331 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT331 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT331>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT331>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT331 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT331> {
    return vec[imagecreate(10, 10),my_func<>];
  }
}
case type CT332 = shape(...)|HH\FunctionRef<(function(): void)>;
              type AliasCT332 = CT332;

  
class CheckAliasCT332<T as AliasCT332> extends BaseCheck {
  const type T = AliasCT332;
  const string NAME = 'AliasCT332';

  <<__LateInit>>
  private AliasCT332 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT332 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT332 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT332>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT332>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT332 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT332> {
    return vec[my_func<>,shape('x' => 10),shape()];
  }
}
case type CT333 = string|HH\FunctionRef<(function(): void)>;
              type AliasCT333 = CT333;

  
class CheckAliasCT333<T as AliasCT333> extends BaseCheck {
  const type T = AliasCT333;
  const string NAME = 'AliasCT333';

  <<__LateInit>>
  private AliasCT333 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT333 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT333 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT333>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT333>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT333 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT333> {
    return vec['','hello world',my_func<>];
  }
}
case type CT334 = vec<mixed>|HH\FunctionRef<(function(): void)>;
              type AliasCT334 = CT334;

  
class CheckAliasCT334<T as AliasCT334> extends BaseCheck {
  const type T = AliasCT334;
  const string NAME = 'AliasCT334';

  <<__LateInit>>
  private AliasCT334 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT334 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT334 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT334>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT334>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT334 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT334> {
    return vec[my_func<>,vec[]];
  }
}
case type CT335 = vec_or_dict<string>|HH\FunctionRef<(function(): void)>;
              type AliasCT335 = CT335;

  
class CheckAliasCT335<T as AliasCT335> extends BaseCheck {
  const type T = AliasCT335;
  const string NAME = 'AliasCT335';

  <<__LateInit>>
  private AliasCT335 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT335 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT335 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT335>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT335>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT335 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT335> {
    return vec[dict[],my_func<>,vec[]];
  }
}
case type CT336 = void|HH\FunctionRef<(function(): void)>;
              type AliasCT336 = CT336;

  
class CheckAliasCT336<T as AliasCT336> extends BaseCheck {
  const type T = AliasCT336;
  const string NAME = 'AliasCT336';

  <<__LateInit>>
  private AliasCT336 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT336 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT336 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT336>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT336>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT336 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT336> {
    return vec[my_func<>,null];
  }
}
case type CT337 = HH\MemberOf<EC, float>|HH\MemberOf<EC, float>;
              type AliasCT337 = CT337;

  
class CheckAliasCT337<T as AliasCT337> extends BaseCheck {
  const type T = AliasCT337;
  const string NAME = 'AliasCT337';

  <<__LateInit>>
  private AliasCT337 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT337 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT337 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT337>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT337>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT337 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT337> {
    return vec[EC::A,EC::B];
  }
}
case type CT338 = I|HH\MemberOf<EC, float>;
              type AliasCT338 = CT338;

  
class CheckAliasCT338<T as AliasCT338> extends BaseCheck {
  const type T = AliasCT338;
  const string NAME = 'AliasCT338';

  <<__LateInit>>
  private AliasCT338 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT338 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT338 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT338>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT338>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT338 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT338> {
    return vec[EC::A,EC::B,new InstanceOfI()];
  }
}
case type CT339 = KeyedContainer<arraykey, mixed>|HH\MemberOf<EC, float>;
              type AliasCT339 = CT339;

  
class CheckAliasCT339<T as AliasCT339> extends BaseCheck {
  const type T = AliasCT339;
  const string NAME = 'AliasCT339';

  <<__LateInit>>
  private AliasCT339 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT339 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT339 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT339>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT339>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT339 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT339> {
    return vec[EC::A,EC::B,dict[],vec[]];
  }
}
case type CT340 = KeyedTraversable<arraykey, mixed>|HH\MemberOf<EC, float>;
              type AliasCT340 = CT340;

  
class CheckAliasCT340<T as AliasCT340> extends BaseCheck {
  const type T = AliasCT340;
  const string NAME = 'AliasCT340';

  <<__LateInit>>
  private AliasCT340 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT340 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT340 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT340>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT340>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT340 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT340> {
    return vec[EC::A,EC::B,dict[],keyset[],vec[]];
  }
}
case type CT341 = MyTrait|HH\MemberOf<EC, float>;
              type AliasCT341 = CT341;

  
class CheckAliasCT341<T as AliasCT341> extends BaseCheck {
  const type T = AliasCT341;
  const string NAME = 'AliasCT341';

  <<__LateInit>>
  private AliasCT341 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT341 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT341 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT341>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT341>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT341 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT341> {
    return vec[EC::A,EC::B];
  }
}
case type CT342 = ReifiedClass<null>|HH\MemberOf<EC, float>;
              type AliasCT342 = CT342;

  
class CheckAliasCT342<T as AliasCT342> extends BaseCheck {
  const type T = AliasCT342;
  const string NAME = 'AliasCT342';

  <<__LateInit>>
  private AliasCT342 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT342 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT342 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT342>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT342>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT342 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT342> {
    return vec[EC::A,EC::B,new ReifiedClass<null>()];
  }
}
case type CT343 = Stringish|HH\MemberOf<EC, float>;
              type AliasCT343 = CT343;

  
class CheckAliasCT343<T as AliasCT343> extends BaseCheck {
  const type T = AliasCT343;
  const string NAME = 'AliasCT343';

  <<__LateInit>>
  private AliasCT343 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT343 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT343 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT343>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT343>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT343 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT343> {
    return vec['','hello world',EC::A,EC::B,new StringishObj()];
  }
}
case type CT344 = Traversable<mixed>|HH\MemberOf<EC, float>;
              type AliasCT344 = CT344;

  
class CheckAliasCT344<T as AliasCT344> extends BaseCheck {
  const type T = AliasCT344;
  const string NAME = 'AliasCT344';

  <<__LateInit>>
  private AliasCT344 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT344 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT344 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT344>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT344>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT344 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT344> {
    return vec[EC::A,EC::B,dict[],keyset[],vec[]];
  }
}
case type CT345 = XHPChild|HH\MemberOf<EC, float>;
              type AliasCT345 = CT345;

  
class CheckAliasCT345<T as AliasCT345> extends BaseCheck {
  const type T = AliasCT345;
  const string NAME = 'AliasCT345';

  <<__LateInit>>
  private AliasCT345 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT345 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT345 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT345>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT345>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT345 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT345> {
    return vec['','hello world',0,1,<my-xhp/>,EC::A,EC::B];
  }
}
case type CT346 = arraykey|HH\MemberOf<EC, float>;
              type AliasCT346 = CT346;

  
class CheckAliasCT346<T as AliasCT346> extends BaseCheck {
  const type T = AliasCT346;
  const string NAME = 'AliasCT346';

  <<__LateInit>>
  private AliasCT346 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT346 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT346 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT346>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT346>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT346 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT346> {
    return vec['','hello world',0,1,EC::A,EC::B];
  }
}
case type CT347 = bool|HH\MemberOf<EC, float>;
              type AliasCT347 = CT347;

  
class CheckAliasCT347<T as AliasCT347> extends BaseCheck {
  const type T = AliasCT347;
  const string NAME = 'AliasCT347';

  <<__LateInit>>
  private AliasCT347 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT347 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT347 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT347>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT347>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT347 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT347> {
    return vec[EC::A,EC::B,false,true];
  }
}
case type CT348 = dict<arraykey, mixed>|HH\MemberOf<EC, float>;
              type AliasCT348 = CT348;

  
class CheckAliasCT348<T as AliasCT348> extends BaseCheck {
  const type T = AliasCT348;
  const string NAME = 'AliasCT348';

  <<__LateInit>>
  private AliasCT348 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT348 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT348 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT348>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT348>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT348 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT348> {
    return vec[EC::A,EC::B,dict[]];
  }
}
case type CT349 = dynamic|HH\MemberOf<EC, float>;
              type AliasCT349 = CT349;

  
class CheckAliasCT349<T as AliasCT349> extends BaseCheck {
  const type T = AliasCT349;
  const string NAME = 'AliasCT349';

  <<__LateInit>>
  private AliasCT349 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT349 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT349 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT349>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT349>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT349 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT349> {
    return vec[EC::A,EC::B,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT350 = float|HH\MemberOf<EC, float>;
              type AliasCT350 = CT350;

  
class CheckAliasCT350<T as AliasCT350> extends BaseCheck {
  const type T = AliasCT350;
  const string NAME = 'AliasCT350';

  <<__LateInit>>
  private AliasCT350 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT350 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT350 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT350>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT350>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT350 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT350> {
    return vec[0.0,3.14,EC::A,EC::B];
  }
}
case type CT351 = int|HH\MemberOf<EC, float>;
              type AliasCT351 = CT351;

  
class CheckAliasCT351<T as AliasCT351> extends BaseCheck {
  const type T = AliasCT351;
  const string NAME = 'AliasCT351';

  <<__LateInit>>
  private AliasCT351 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT351 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT351 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT351>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT351>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT351 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT351> {
    return vec[0,1,EC::A,EC::B];
  }
}
case type CT352 = keyset<arraykey>|HH\MemberOf<EC, float>;
              type AliasCT352 = CT352;

  
class CheckAliasCT352<T as AliasCT352> extends BaseCheck {
  const type T = AliasCT352;
  const string NAME = 'AliasCT352';

  <<__LateInit>>
  private AliasCT352 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT352 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT352 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT352>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT352>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT352 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT352> {
    return vec[EC::A,EC::B,keyset[]];
  }
}
case type CT353 = mixed|HH\MemberOf<EC, float>;
              type AliasCT353 = CT353;

  
class CheckAliasCT353<T as AliasCT353> extends BaseCheck {
  const type T = AliasCT353;
  const string NAME = 'AliasCT353';

  <<__LateInit>>
  private AliasCT353 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT353 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT353 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT353>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT353>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT353 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT353> {
    return vec['','hello world',0,1,EC::A,EC::B,false,null,true];
  }
}
case type CT354 = nonnull|HH\MemberOf<EC, float>;
              type AliasCT354 = CT354;

  
class CheckAliasCT354<T as AliasCT354> extends BaseCheck {
  const type T = AliasCT354;
  const string NAME = 'AliasCT354';

  <<__LateInit>>
  private AliasCT354 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT354 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT354 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT354>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT354>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT354 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT354> {
    return vec['','hello world',0,1,EC::A,EC::B,false,true];
  }
}
case type CT355 = noreturn|HH\MemberOf<EC, float>;
              type AliasCT355 = CT355;

  
class CheckAliasCT355<T as AliasCT355> extends BaseCheck {
  const type T = AliasCT355;
  const string NAME = 'AliasCT355';

  <<__LateInit>>
  private AliasCT355 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT355 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT355 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT355>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT355>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT355 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT355> {
    return vec[EC::A,EC::B];
  }
}
case type CT356 = nothing|HH\MemberOf<EC, float>;
              type AliasCT356 = CT356;

  
class CheckAliasCT356<T as AliasCT356> extends BaseCheck {
  const type T = AliasCT356;
  const string NAME = 'AliasCT356';

  <<__LateInit>>
  private AliasCT356 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT356 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT356 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT356>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT356>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT356 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT356> {
    return vec[EC::A,EC::B];
  }
}
case type CT357 = null|HH\MemberOf<EC, float>;
              type AliasCT357 = CT357;

  
class CheckAliasCT357<T as AliasCT357> extends BaseCheck {
  const type T = AliasCT357;
  const string NAME = 'AliasCT357';

  <<__LateInit>>
  private AliasCT357 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT357 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT357 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT357>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT357>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT357 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT357> {
    return vec[EC::A,EC::B,null];
  }
}
case type CT358 = num|HH\MemberOf<EC, float>;
              type AliasCT358 = CT358;

  
class CheckAliasCT358<T as AliasCT358> extends BaseCheck {
  const type T = AliasCT358;
  const string NAME = 'AliasCT358';

  <<__LateInit>>
  private AliasCT358 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT358 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT358 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT358>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT358>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT358 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT358> {
    return vec[0,0.0,1,3.14,EC::A,EC::B];
  }
}
case type CT359 = resource|HH\MemberOf<EC, float>;
              type AliasCT359 = CT359;

  
class CheckAliasCT359<T as AliasCT359> extends BaseCheck {
  const type T = AliasCT359;
  const string NAME = 'AliasCT359';

  <<__LateInit>>
  private AliasCT359 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT359 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT359 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT359>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT359>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT359 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT359> {
    return vec[EC::A,EC::B,imagecreate(10, 10)];
  }
}
case type CT360 = shape(...)|HH\MemberOf<EC, float>;
              type AliasCT360 = CT360;

  
class CheckAliasCT360<T as AliasCT360> extends BaseCheck {
  const type T = AliasCT360;
  const string NAME = 'AliasCT360';

  <<__LateInit>>
  private AliasCT360 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT360 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT360 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT360>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT360>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT360 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT360> {
    return vec[EC::A,EC::B,shape('x' => 10),shape()];
  }
}
case type CT361 = string|HH\MemberOf<EC, float>;
              type AliasCT361 = CT361;

  
class CheckAliasCT361<T as AliasCT361> extends BaseCheck {
  const type T = AliasCT361;
  const string NAME = 'AliasCT361';

  <<__LateInit>>
  private AliasCT361 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT361 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT361 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT361>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT361>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT361 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT361> {
    return vec['','hello world',EC::A,EC::B];
  }
}
case type CT362 = vec<mixed>|HH\MemberOf<EC, float>;
              type AliasCT362 = CT362;

  
class CheckAliasCT362<T as AliasCT362> extends BaseCheck {
  const type T = AliasCT362;
  const string NAME = 'AliasCT362';

  <<__LateInit>>
  private AliasCT362 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT362 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT362 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT362>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT362>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT362 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT362> {
    return vec[EC::A,EC::B,vec[]];
  }
}
case type CT363 = vec_or_dict<string>|HH\MemberOf<EC, float>;
              type AliasCT363 = CT363;

  
class CheckAliasCT363<T as AliasCT363> extends BaseCheck {
  const type T = AliasCT363;
  const string NAME = 'AliasCT363';

  <<__LateInit>>
  private AliasCT363 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT363 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT363 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT363>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT363>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT363 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT363> {
    return vec[EC::A,EC::B,dict[],vec[]];
  }
}
case type CT364 = void|HH\MemberOf<EC, float>;
              type AliasCT364 = CT364;

  
class CheckAliasCT364<T as AliasCT364> extends BaseCheck {
  const type T = AliasCT364;
  const string NAME = 'AliasCT364';

  <<__LateInit>>
  private AliasCT364 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT364 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT364 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT364>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT364>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT364 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT364> {
    return vec[EC::A,EC::B,null];
  }
}
case type CT365 = I|I;
              type AliasCT365 = CT365;

  
class CheckAliasCT365<T as AliasCT365> extends BaseCheck {
  const type T = AliasCT365;
  const string NAME = 'AliasCT365';

  <<__LateInit>>
  private AliasCT365 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT365 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT365 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT365>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT365>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT365 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT365> {
    return vec[new InstanceOfI()];
  }
}
case type CT366 = KeyedContainer<arraykey, mixed>|I;
              type AliasCT366 = CT366;

  
class CheckAliasCT366<T as AliasCT366> extends BaseCheck {
  const type T = AliasCT366;
  const string NAME = 'AliasCT366';

  <<__LateInit>>
  private AliasCT366 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT366 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT366 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT366>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT366>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT366 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT366> {
    return vec[dict[],new InstanceOfI(),vec[]];
  }
}
case type CT367 = KeyedTraversable<arraykey, mixed>|I;
              type AliasCT367 = CT367;

  
class CheckAliasCT367<T as AliasCT367> extends BaseCheck {
  const type T = AliasCT367;
  const string NAME = 'AliasCT367';

  <<__LateInit>>
  private AliasCT367 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT367 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT367 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT367>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT367>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT367 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT367> {
    return vec[dict[],keyset[],new InstanceOfI(),vec[]];
  }
}
case type CT368 = MyTrait|I;
              type AliasCT368 = CT368;

  
class CheckAliasCT368<T as AliasCT368> extends BaseCheck {
  const type T = AliasCT368;
  const string NAME = 'AliasCT368';

  <<__LateInit>>
  private AliasCT368 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT368 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT368 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT368>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT368>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT368 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT368> {
    return vec[new InstanceOfI()];
  }
}
case type CT369 = ReifiedClass<null>|I;
              type AliasCT369 = CT369;

  
class CheckAliasCT369<T as AliasCT369> extends BaseCheck {
  const type T = AliasCT369;
  const string NAME = 'AliasCT369';

  <<__LateInit>>
  private AliasCT369 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT369 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT369 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT369>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT369>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT369 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT369> {
    return vec[new InstanceOfI(),new ReifiedClass<null>()];
  }
}
case type CT370 = Stringish|I;
              type AliasCT370 = CT370;

  
class CheckAliasCT370<T as AliasCT370> extends BaseCheck {
  const type T = AliasCT370;
  const string NAME = 'AliasCT370';

  <<__LateInit>>
  private AliasCT370 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT370 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT370 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT370>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT370>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT370 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT370> {
    return vec['','hello world',new InstanceOfI(),new StringishObj()];
  }
}
case type CT371 = Traversable<mixed>|I;
              type AliasCT371 = CT371;

  
class CheckAliasCT371<T as AliasCT371> extends BaseCheck {
  const type T = AliasCT371;
  const string NAME = 'AliasCT371';

  <<__LateInit>>
  private AliasCT371 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT371 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT371 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT371>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT371>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT371 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT371> {
    return vec[dict[],keyset[],new InstanceOfI(),vec[]];
  }
}
case type CT372 = XHPChild|I;
              type AliasCT372 = CT372;

  
class CheckAliasCT372<T as AliasCT372> extends BaseCheck {
  const type T = AliasCT372;
  const string NAME = 'AliasCT372';

  <<__LateInit>>
  private AliasCT372 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT372 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT372 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT372>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT372>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT372 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT372> {
    return vec['','hello world',0,1,<my-xhp/>,new InstanceOfI()];
  }
}
case type CT373 = arraykey|I;
              type AliasCT373 = CT373;

  
class CheckAliasCT373<T as AliasCT373> extends BaseCheck {
  const type T = AliasCT373;
  const string NAME = 'AliasCT373';

  <<__LateInit>>
  private AliasCT373 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT373 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT373 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT373>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT373>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT373 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT373> {
    return vec['','hello world',0,1,new InstanceOfI()];
  }
}
case type CT374 = bool|I;
              type AliasCT374 = CT374;

  
class CheckAliasCT374<T as AliasCT374> extends BaseCheck {
  const type T = AliasCT374;
  const string NAME = 'AliasCT374';

  <<__LateInit>>
  private AliasCT374 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT374 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT374 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT374>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT374>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT374 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT374> {
    return vec[false,new InstanceOfI(),true];
  }
}
case type CT375 = dict<arraykey, mixed>|I;
              type AliasCT375 = CT375;

  
class CheckAliasCT375<T as AliasCT375> extends BaseCheck {
  const type T = AliasCT375;
  const string NAME = 'AliasCT375';

  <<__LateInit>>
  private AliasCT375 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT375 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT375 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT375>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT375>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT375 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT375> {
    return vec[dict[],new InstanceOfI()];
  }
}
case type CT376 = dynamic|I;
              type AliasCT376 = CT376;

  
class CheckAliasCT376<T as AliasCT376> extends BaseCheck {
  const type T = AliasCT376;
  const string NAME = 'AliasCT376';

  <<__LateInit>>
  private AliasCT376 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT376 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT376 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT376>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT376>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT376 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT376> {
    return vec[false,new InstanceOfI(),null,shape('x' => 10),shape(),true];
  }
}
case type CT377 = float|I;
              type AliasCT377 = CT377;

  
class CheckAliasCT377<T as AliasCT377> extends BaseCheck {
  const type T = AliasCT377;
  const string NAME = 'AliasCT377';

  <<__LateInit>>
  private AliasCT377 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT377 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT377 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT377>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT377>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT377 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT377> {
    return vec[0.0,3.14,new InstanceOfI()];
  }
}
case type CT378 = int|I;
              type AliasCT378 = CT378;

  
class CheckAliasCT378<T as AliasCT378> extends BaseCheck {
  const type T = AliasCT378;
  const string NAME = 'AliasCT378';

  <<__LateInit>>
  private AliasCT378 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT378 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT378 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT378>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT378>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT378 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT378> {
    return vec[0,1,new InstanceOfI()];
  }
}
case type CT379 = keyset<arraykey>|I;
              type AliasCT379 = CT379;

  
class CheckAliasCT379<T as AliasCT379> extends BaseCheck {
  const type T = AliasCT379;
  const string NAME = 'AliasCT379';

  <<__LateInit>>
  private AliasCT379 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT379 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT379 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT379>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT379>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT379 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT379> {
    return vec[keyset[],new InstanceOfI()];
  }
}
case type CT380 = mixed|I;
              type AliasCT380 = CT380;

  
class CheckAliasCT380<T as AliasCT380> extends BaseCheck {
  const type T = AliasCT380;
  const string NAME = 'AliasCT380';

  <<__LateInit>>
  private AliasCT380 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT380 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT380 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT380>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT380>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT380 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT380> {
    return vec['','hello world',0,1,false,new InstanceOfI(),null,true];
  }
}
case type CT381 = nonnull|I;
              type AliasCT381 = CT381;

  
class CheckAliasCT381<T as AliasCT381> extends BaseCheck {
  const type T = AliasCT381;
  const string NAME = 'AliasCT381';

  <<__LateInit>>
  private AliasCT381 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT381 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT381 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT381>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT381>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT381 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT381> {
    return vec['','hello world',0,1,false,new InstanceOfI(),true];
  }
}
case type CT382 = noreturn|I;
              type AliasCT382 = CT382;

  
class CheckAliasCT382<T as AliasCT382> extends BaseCheck {
  const type T = AliasCT382;
  const string NAME = 'AliasCT382';

  <<__LateInit>>
  private AliasCT382 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT382 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT382 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT382>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT382>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT382 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT382> {
    return vec[new InstanceOfI()];
  }
}
case type CT383 = nothing|I;
              type AliasCT383 = CT383;

  
class CheckAliasCT383<T as AliasCT383> extends BaseCheck {
  const type T = AliasCT383;
  const string NAME = 'AliasCT383';

  <<__LateInit>>
  private AliasCT383 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT383 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT383 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT383>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT383>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT383 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT383> {
    return vec[new InstanceOfI()];
  }
}
case type CT384 = null|I;
              type AliasCT384 = CT384;

  
class CheckAliasCT384<T as AliasCT384> extends BaseCheck {
  const type T = AliasCT384;
  const string NAME = 'AliasCT384';

  <<__LateInit>>
  private AliasCT384 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT384 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT384 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT384>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT384>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT384 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT384> {
    return vec[new InstanceOfI(),null];
  }
}
case type CT385 = num|I;
              type AliasCT385 = CT385;

  
class CheckAliasCT385<T as AliasCT385> extends BaseCheck {
  const type T = AliasCT385;
  const string NAME = 'AliasCT385';

  <<__LateInit>>
  private AliasCT385 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT385 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT385 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT385>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT385>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT385 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT385> {
    return vec[0,0.0,1,3.14,new InstanceOfI()];
  }
}
case type CT386 = resource|I;
              type AliasCT386 = CT386;

  
class CheckAliasCT386<T as AliasCT386> extends BaseCheck {
  const type T = AliasCT386;
  const string NAME = 'AliasCT386';

  <<__LateInit>>
  private AliasCT386 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT386 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT386 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT386>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT386>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT386 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT386> {
    return vec[imagecreate(10, 10),new InstanceOfI()];
  }
}
case type CT387 = shape(...)|I;
              type AliasCT387 = CT387;

  
class CheckAliasCT387<T as AliasCT387> extends BaseCheck {
  const type T = AliasCT387;
  const string NAME = 'AliasCT387';

  <<__LateInit>>
  private AliasCT387 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT387 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT387 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT387>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT387>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT387 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT387> {
    return vec[new InstanceOfI(),shape('x' => 10),shape()];
  }
}
case type CT388 = string|I;
              type AliasCT388 = CT388;

  
class CheckAliasCT388<T as AliasCT388> extends BaseCheck {
  const type T = AliasCT388;
  const string NAME = 'AliasCT388';

  <<__LateInit>>
  private AliasCT388 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT388 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT388 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT388>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT388>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT388 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT388> {
    return vec['','hello world',new InstanceOfI()];
  }
}
case type CT389 = vec<mixed>|I;
              type AliasCT389 = CT389;

  
class CheckAliasCT389<T as AliasCT389> extends BaseCheck {
  const type T = AliasCT389;
  const string NAME = 'AliasCT389';

  <<__LateInit>>
  private AliasCT389 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT389 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT389 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT389>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT389>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT389 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT389> {
    return vec[new InstanceOfI(),vec[]];
  }
}
case type CT390 = vec_or_dict<string>|I;
              type AliasCT390 = CT390;

  
class CheckAliasCT390<T as AliasCT390> extends BaseCheck {
  const type T = AliasCT390;
  const string NAME = 'AliasCT390';

  <<__LateInit>>
  private AliasCT390 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT390 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT390 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT390>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT390>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT390 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT390> {
    return vec[dict[],new InstanceOfI(),vec[]];
  }
}
case type CT391 = void|I;
              type AliasCT391 = CT391;

  
class CheckAliasCT391<T as AliasCT391> extends BaseCheck {
  const type T = AliasCT391;
  const string NAME = 'AliasCT391';

  <<__LateInit>>
  private AliasCT391 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT391 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT391 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT391>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT391>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT391 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT391> {
    return vec[new InstanceOfI(),null];
  }
}
case type CT392 = KeyedContainer<arraykey, mixed>|KeyedContainer<arraykey, mixed>;
              type AliasCT392 = CT392;

  
class CheckAliasCT392<T as AliasCT392> extends BaseCheck {
  const type T = AliasCT392;
  const string NAME = 'AliasCT392';

  <<__LateInit>>
  private AliasCT392 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT392 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT392 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT392>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT392>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT392 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT392> {
    return vec[dict[],vec[]];
  }
}
case type CT393 = KeyedTraversable<arraykey, mixed>|KeyedContainer<arraykey, mixed>;
              type AliasCT393 = CT393;

  
class CheckAliasCT393<T as AliasCT393> extends BaseCheck {
  const type T = AliasCT393;
  const string NAME = 'AliasCT393';

  <<__LateInit>>
  private AliasCT393 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT393 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT393 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT393>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT393>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT393 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT393> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT394 = MyTrait|KeyedContainer<arraykey, mixed>;
              type AliasCT394 = CT394;

  
class CheckAliasCT394<T as AliasCT394> extends BaseCheck {
  const type T = AliasCT394;
  const string NAME = 'AliasCT394';

  <<__LateInit>>
  private AliasCT394 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT394 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT394 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT394>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT394>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT394 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT394> {
    return vec[dict[],vec[]];
  }
}
case type CT395 = ReifiedClass<null>|KeyedContainer<arraykey, mixed>;
              type AliasCT395 = CT395;

  
class CheckAliasCT395<T as AliasCT395> extends BaseCheck {
  const type T = AliasCT395;
  const string NAME = 'AliasCT395';

  <<__LateInit>>
  private AliasCT395 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT395 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT395 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT395>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT395>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT395 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT395> {
    return vec[dict[],new ReifiedClass<null>(),vec[]];
  }
}
case type CT396 = Stringish|KeyedContainer<arraykey, mixed>;
              type AliasCT396 = CT396;

  
class CheckAliasCT396<T as AliasCT396> extends BaseCheck {
  const type T = AliasCT396;
  const string NAME = 'AliasCT396';

  <<__LateInit>>
  private AliasCT396 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT396 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT396 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT396>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT396>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT396 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT396> {
    return vec['','hello world',dict[],new StringishObj(),vec[]];
  }
}
case type CT397 = Traversable<mixed>|KeyedContainer<arraykey, mixed>;
              type AliasCT397 = CT397;

  
class CheckAliasCT397<T as AliasCT397> extends BaseCheck {
  const type T = AliasCT397;
  const string NAME = 'AliasCT397';

  <<__LateInit>>
  private AliasCT397 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT397 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT397 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT397>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT397>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT397 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT397> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT398 = XHPChild|KeyedContainer<arraykey, mixed>;
              type AliasCT398 = CT398;

  
class CheckAliasCT398<T as AliasCT398> extends BaseCheck {
  const type T = AliasCT398;
  const string NAME = 'AliasCT398';

  <<__LateInit>>
  private AliasCT398 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT398 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT398 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT398>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT398>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT398 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT398> {
    return vec['','hello world',0,1,<my-xhp/>,dict[],vec[]];
  }
}
case type CT399 = arraykey|KeyedContainer<arraykey, mixed>;
              type AliasCT399 = CT399;

  
class CheckAliasCT399<T as AliasCT399> extends BaseCheck {
  const type T = AliasCT399;
  const string NAME = 'AliasCT399';

  <<__LateInit>>
  private AliasCT399 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT399 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT399 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT399>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT399>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT399 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT399> {
    return vec['','hello world',0,1,dict[],vec[]];
  }
}
case type CT400 = bool|KeyedContainer<arraykey, mixed>;
              type AliasCT400 = CT400;

  
class CheckAliasCT400<T as AliasCT400> extends BaseCheck {
  const type T = AliasCT400;
  const string NAME = 'AliasCT400';

  <<__LateInit>>
  private AliasCT400 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT400 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT400 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT400>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT400>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT400 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT400> {
    return vec[dict[],false,true,vec[]];
  }
}
case type CT401 = dict<arraykey, mixed>|KeyedContainer<arraykey, mixed>;
              type AliasCT401 = CT401;

  
class CheckAliasCT401<T as AliasCT401> extends BaseCheck {
  const type T = AliasCT401;
  const string NAME = 'AliasCT401';

  <<__LateInit>>
  private AliasCT401 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT401 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT401 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT401>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT401>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT401 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT401> {
    return vec[dict[],vec[]];
  }
}
case type CT402 = dynamic|KeyedContainer<arraykey, mixed>;
              type AliasCT402 = CT402;

  
class CheckAliasCT402<T as AliasCT402> extends BaseCheck {
  const type T = AliasCT402;
  const string NAME = 'AliasCT402';

  <<__LateInit>>
  private AliasCT402 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT402 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT402 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT402>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT402>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT402 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT402> {
    return vec[dict[],false,null,shape('x' => 10),shape(),true,vec[]];
  }
}
case type CT403 = float|KeyedContainer<arraykey, mixed>;
              type AliasCT403 = CT403;

  
class CheckAliasCT403<T as AliasCT403> extends BaseCheck {
  const type T = AliasCT403;
  const string NAME = 'AliasCT403';

  <<__LateInit>>
  private AliasCT403 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT403 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT403 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT403>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT403>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT403 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT403> {
    return vec[0.0,3.14,dict[],vec[]];
  }
}
case type CT404 = int|KeyedContainer<arraykey, mixed>;
              type AliasCT404 = CT404;

  
class CheckAliasCT404<T as AliasCT404> extends BaseCheck {
  const type T = AliasCT404;
  const string NAME = 'AliasCT404';

  <<__LateInit>>
  private AliasCT404 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT404 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT404 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT404>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT404>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT404 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT404> {
    return vec[0,1,dict[],vec[]];
  }
}
case type CT405 = keyset<arraykey>|KeyedContainer<arraykey, mixed>;
              type AliasCT405 = CT405;

  
class CheckAliasCT405<T as AliasCT405> extends BaseCheck {
  const type T = AliasCT405;
  const string NAME = 'AliasCT405';

  <<__LateInit>>
  private AliasCT405 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT405 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT405 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT405>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT405>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT405 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT405> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT406 = mixed|KeyedContainer<arraykey, mixed>;
              type AliasCT406 = CT406;

  
class CheckAliasCT406<T as AliasCT406> extends BaseCheck {
  const type T = AliasCT406;
  const string NAME = 'AliasCT406';

  <<__LateInit>>
  private AliasCT406 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT406 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT406 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT406>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT406>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT406 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT406> {
    return vec['','hello world',0,1,dict[],false,null,true,vec[]];
  }
}
case type CT407 = nonnull|KeyedContainer<arraykey, mixed>;
              type AliasCT407 = CT407;

  
class CheckAliasCT407<T as AliasCT407> extends BaseCheck {
  const type T = AliasCT407;
  const string NAME = 'AliasCT407';

  <<__LateInit>>
  private AliasCT407 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT407 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT407 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT407>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT407>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT407 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT407> {
    return vec['','hello world',0,1,dict[],false,true,vec[]];
  }
}
case type CT408 = noreturn|KeyedContainer<arraykey, mixed>;
              type AliasCT408 = CT408;

  
class CheckAliasCT408<T as AliasCT408> extends BaseCheck {
  const type T = AliasCT408;
  const string NAME = 'AliasCT408';

  <<__LateInit>>
  private AliasCT408 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT408 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT408 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT408>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT408>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT408 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT408> {
    return vec[dict[],vec[]];
  }
}
case type CT409 = nothing|KeyedContainer<arraykey, mixed>;
              type AliasCT409 = CT409;

  
class CheckAliasCT409<T as AliasCT409> extends BaseCheck {
  const type T = AliasCT409;
  const string NAME = 'AliasCT409';

  <<__LateInit>>
  private AliasCT409 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT409 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT409 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT409>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT409>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT409 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT409> {
    return vec[dict[],vec[]];
  }
}
case type CT410 = null|KeyedContainer<arraykey, mixed>;
              type AliasCT410 = CT410;

  
class CheckAliasCT410<T as AliasCT410> extends BaseCheck {
  const type T = AliasCT410;
  const string NAME = 'AliasCT410';

  <<__LateInit>>
  private AliasCT410 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT410 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT410 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT410>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT410>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT410 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT410> {
    return vec[dict[],null,vec[]];
  }
}
case type CT411 = num|KeyedContainer<arraykey, mixed>;
              type AliasCT411 = CT411;

  
class CheckAliasCT411<T as AliasCT411> extends BaseCheck {
  const type T = AliasCT411;
  const string NAME = 'AliasCT411';

  <<__LateInit>>
  private AliasCT411 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT411 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT411 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT411>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT411>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT411 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT411> {
    return vec[0,0.0,1,3.14,dict[],vec[]];
  }
}
case type CT412 = resource|KeyedContainer<arraykey, mixed>;
              type AliasCT412 = CT412;

  
class CheckAliasCT412<T as AliasCT412> extends BaseCheck {
  const type T = AliasCT412;
  const string NAME = 'AliasCT412';

  <<__LateInit>>
  private AliasCT412 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT412 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT412 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT412>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT412>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT412 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT412> {
    return vec[dict[],imagecreate(10, 10),vec[]];
  }
}
case type CT413 = shape(...)|KeyedContainer<arraykey, mixed>;
              type AliasCT413 = CT413;

  
class CheckAliasCT413<T as AliasCT413> extends BaseCheck {
  const type T = AliasCT413;
  const string NAME = 'AliasCT413';

  <<__LateInit>>
  private AliasCT413 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT413 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT413 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT413>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT413>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT413 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT413> {
    return vec[dict[],shape('x' => 10),shape(),vec[]];
  }
}
case type CT414 = string|KeyedContainer<arraykey, mixed>;
              type AliasCT414 = CT414;

  
class CheckAliasCT414<T as AliasCT414> extends BaseCheck {
  const type T = AliasCT414;
  const string NAME = 'AliasCT414';

  <<__LateInit>>
  private AliasCT414 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT414 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT414 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT414>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT414>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT414 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT414> {
    return vec['','hello world',dict[],vec[]];
  }
}
case type CT415 = vec<mixed>|KeyedContainer<arraykey, mixed>;
              type AliasCT415 = CT415;

  
class CheckAliasCT415<T as AliasCT415> extends BaseCheck {
  const type T = AliasCT415;
  const string NAME = 'AliasCT415';

  <<__LateInit>>
  private AliasCT415 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT415 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT415 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT415>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT415>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT415 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT415> {
    return vec[dict[],vec[]];
  }
}
case type CT416 = vec_or_dict<string>|KeyedContainer<arraykey, mixed>;
              type AliasCT416 = CT416;

  
class CheckAliasCT416<T as AliasCT416> extends BaseCheck {
  const type T = AliasCT416;
  const string NAME = 'AliasCT416';

  <<__LateInit>>
  private AliasCT416 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT416 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT416 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT416>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT416>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT416 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT416> {
    return vec[dict[],vec[]];
  }
}
case type CT417 = void|KeyedContainer<arraykey, mixed>;
              type AliasCT417 = CT417;

  
class CheckAliasCT417<T as AliasCT417> extends BaseCheck {
  const type T = AliasCT417;
  const string NAME = 'AliasCT417';

  <<__LateInit>>
  private AliasCT417 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT417 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT417 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT417>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT417>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT417 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT417> {
    return vec[dict[],null,vec[]];
  }
}
case type CT418 = KeyedTraversable<arraykey, mixed>|KeyedTraversable<arraykey, mixed>;
              type AliasCT418 = CT418;

  
class CheckAliasCT418<T as AliasCT418> extends BaseCheck {
  const type T = AliasCT418;
  const string NAME = 'AliasCT418';

  <<__LateInit>>
  private AliasCT418 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT418 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT418 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT418>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT418>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT418 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT418> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT419 = MyTrait|KeyedTraversable<arraykey, mixed>;
              type AliasCT419 = CT419;

  
class CheckAliasCT419<T as AliasCT419> extends BaseCheck {
  const type T = AliasCT419;
  const string NAME = 'AliasCT419';

  <<__LateInit>>
  private AliasCT419 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT419 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT419 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT419>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT419>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT419 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT419> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT420 = ReifiedClass<null>|KeyedTraversable<arraykey, mixed>;
              type AliasCT420 = CT420;

  
class CheckAliasCT420<T as AliasCT420> extends BaseCheck {
  const type T = AliasCT420;
  const string NAME = 'AliasCT420';

  <<__LateInit>>
  private AliasCT420 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT420 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT420 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT420>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT420>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT420 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT420> {
    return vec[dict[],keyset[],new ReifiedClass<null>(),vec[]];
  }
}
case type CT421 = Stringish|KeyedTraversable<arraykey, mixed>;
              type AliasCT421 = CT421;

  
class CheckAliasCT421<T as AliasCT421> extends BaseCheck {
  const type T = AliasCT421;
  const string NAME = 'AliasCT421';

  <<__LateInit>>
  private AliasCT421 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT421 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT421 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT421>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT421>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT421 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT421> {
    return vec['','hello world',dict[],keyset[],new StringishObj(),vec[]];
  }
}
case type CT422 = Traversable<mixed>|KeyedTraversable<arraykey, mixed>;
              type AliasCT422 = CT422;

  
class CheckAliasCT422<T as AliasCT422> extends BaseCheck {
  const type T = AliasCT422;
  const string NAME = 'AliasCT422';

  <<__LateInit>>
  private AliasCT422 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT422 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT422 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT422>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT422>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT422 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT422> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT423 = XHPChild|KeyedTraversable<arraykey, mixed>;
              type AliasCT423 = CT423;

  
class CheckAliasCT423<T as AliasCT423> extends BaseCheck {
  const type T = AliasCT423;
  const string NAME = 'AliasCT423';

  <<__LateInit>>
  private AliasCT423 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT423 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT423 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT423>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT423>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT423 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT423> {
    return vec['','hello world',0,1,<my-xhp/>,dict[],keyset[],vec[]];
  }
}
case type CT424 = arraykey|KeyedTraversable<arraykey, mixed>;
              type AliasCT424 = CT424;

  
class CheckAliasCT424<T as AliasCT424> extends BaseCheck {
  const type T = AliasCT424;
  const string NAME = 'AliasCT424';

  <<__LateInit>>
  private AliasCT424 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT424 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT424 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT424>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT424>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT424 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT424> {
    return vec['','hello world',0,1,dict[],keyset[],vec[]];
  }
}
case type CT425 = bool|KeyedTraversable<arraykey, mixed>;
              type AliasCT425 = CT425;

  
class CheckAliasCT425<T as AliasCT425> extends BaseCheck {
  const type T = AliasCT425;
  const string NAME = 'AliasCT425';

  <<__LateInit>>
  private AliasCT425 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT425 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT425 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT425>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT425>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT425 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT425> {
    return vec[dict[],false,keyset[],true,vec[]];
  }
}
case type CT426 = dict<arraykey, mixed>|KeyedTraversable<arraykey, mixed>;
              type AliasCT426 = CT426;

  
class CheckAliasCT426<T as AliasCT426> extends BaseCheck {
  const type T = AliasCT426;
  const string NAME = 'AliasCT426';

  <<__LateInit>>
  private AliasCT426 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT426 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT426 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT426>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT426>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT426 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT426> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT427 = dynamic|KeyedTraversable<arraykey, mixed>;
              type AliasCT427 = CT427;

  
class CheckAliasCT427<T as AliasCT427> extends BaseCheck {
  const type T = AliasCT427;
  const string NAME = 'AliasCT427';

  <<__LateInit>>
  private AliasCT427 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT427 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT427 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT427>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT427>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT427 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT427> {
    return vec[dict[],false,keyset[],null,shape('x' => 10),shape(),true,vec[]];
  }
}
case type CT428 = float|KeyedTraversable<arraykey, mixed>;
              type AliasCT428 = CT428;

  
class CheckAliasCT428<T as AliasCT428> extends BaseCheck {
  const type T = AliasCT428;
  const string NAME = 'AliasCT428';

  <<__LateInit>>
  private AliasCT428 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT428 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT428 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT428>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT428>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT428 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT428> {
    return vec[0.0,3.14,dict[],keyset[],vec[]];
  }
}
case type CT429 = int|KeyedTraversable<arraykey, mixed>;
              type AliasCT429 = CT429;

  
class CheckAliasCT429<T as AliasCT429> extends BaseCheck {
  const type T = AliasCT429;
  const string NAME = 'AliasCT429';

  <<__LateInit>>
  private AliasCT429 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT429 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT429 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT429>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT429>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT429 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT429> {
    return vec[0,1,dict[],keyset[],vec[]];
  }
}
case type CT430 = keyset<arraykey>|KeyedTraversable<arraykey, mixed>;
              type AliasCT430 = CT430;

  
class CheckAliasCT430<T as AliasCT430> extends BaseCheck {
  const type T = AliasCT430;
  const string NAME = 'AliasCT430';

  <<__LateInit>>
  private AliasCT430 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT430 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT430 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT430>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT430>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT430 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT430> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT431 = mixed|KeyedTraversable<arraykey, mixed>;
              type AliasCT431 = CT431;

  
class CheckAliasCT431<T as AliasCT431> extends BaseCheck {
  const type T = AliasCT431;
  const string NAME = 'AliasCT431';

  <<__LateInit>>
  private AliasCT431 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT431 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT431 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT431>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT431>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT431 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT431> {
    return vec['','hello world',0,1,dict[],false,keyset[],null,true,vec[]];
  }
}
case type CT432 = nonnull|KeyedTraversable<arraykey, mixed>;
              type AliasCT432 = CT432;

  
class CheckAliasCT432<T as AliasCT432> extends BaseCheck {
  const type T = AliasCT432;
  const string NAME = 'AliasCT432';

  <<__LateInit>>
  private AliasCT432 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT432 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT432 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT432>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT432>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT432 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT432> {
    return vec['','hello world',0,1,dict[],false,keyset[],true,vec[]];
  }
}
case type CT433 = noreturn|KeyedTraversable<arraykey, mixed>;
              type AliasCT433 = CT433;

  
class CheckAliasCT433<T as AliasCT433> extends BaseCheck {
  const type T = AliasCT433;
  const string NAME = 'AliasCT433';

  <<__LateInit>>
  private AliasCT433 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT433 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT433 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT433>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT433>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT433 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT433> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT434 = nothing|KeyedTraversable<arraykey, mixed>;
              type AliasCT434 = CT434;

  
class CheckAliasCT434<T as AliasCT434> extends BaseCheck {
  const type T = AliasCT434;
  const string NAME = 'AliasCT434';

  <<__LateInit>>
  private AliasCT434 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT434 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT434 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT434>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT434>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT434 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT434> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT435 = null|KeyedTraversable<arraykey, mixed>;
              type AliasCT435 = CT435;

  
class CheckAliasCT435<T as AliasCT435> extends BaseCheck {
  const type T = AliasCT435;
  const string NAME = 'AliasCT435';

  <<__LateInit>>
  private AliasCT435 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT435 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT435 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT435>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT435>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT435 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT435> {
    return vec[dict[],keyset[],null,vec[]];
  }
}
case type CT436 = num|KeyedTraversable<arraykey, mixed>;
              type AliasCT436 = CT436;

  
class CheckAliasCT436<T as AliasCT436> extends BaseCheck {
  const type T = AliasCT436;
  const string NAME = 'AliasCT436';

  <<__LateInit>>
  private AliasCT436 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT436 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT436 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT436>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT436>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT436 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT436> {
    return vec[0,0.0,1,3.14,dict[],keyset[],vec[]];
  }
}
case type CT437 = resource|KeyedTraversable<arraykey, mixed>;
              type AliasCT437 = CT437;

  
class CheckAliasCT437<T as AliasCT437> extends BaseCheck {
  const type T = AliasCT437;
  const string NAME = 'AliasCT437';

  <<__LateInit>>
  private AliasCT437 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT437 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT437 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT437>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT437>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT437 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT437> {
    return vec[dict[],imagecreate(10, 10),keyset[],vec[]];
  }
}
case type CT438 = shape(...)|KeyedTraversable<arraykey, mixed>;
              type AliasCT438 = CT438;

  
class CheckAliasCT438<T as AliasCT438> extends BaseCheck {
  const type T = AliasCT438;
  const string NAME = 'AliasCT438';

  <<__LateInit>>
  private AliasCT438 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT438 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT438 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT438>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT438>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT438 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT438> {
    return vec[dict[],keyset[],shape('x' => 10),shape(),vec[]];
  }
}
case type CT439 = string|KeyedTraversable<arraykey, mixed>;
              type AliasCT439 = CT439;

  
class CheckAliasCT439<T as AliasCT439> extends BaseCheck {
  const type T = AliasCT439;
  const string NAME = 'AliasCT439';

  <<__LateInit>>
  private AliasCT439 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT439 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT439 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT439>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT439>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT439 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT439> {
    return vec['','hello world',dict[],keyset[],vec[]];
  }
}
case type CT440 = vec<mixed>|KeyedTraversable<arraykey, mixed>;
              type AliasCT440 = CT440;

  
class CheckAliasCT440<T as AliasCT440> extends BaseCheck {
  const type T = AliasCT440;
  const string NAME = 'AliasCT440';

  <<__LateInit>>
  private AliasCT440 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT440 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT440 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT440>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT440>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT440 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT440> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT441 = vec_or_dict<string>|KeyedTraversable<arraykey, mixed>;
              type AliasCT441 = CT441;

  
class CheckAliasCT441<T as AliasCT441> extends BaseCheck {
  const type T = AliasCT441;
  const string NAME = 'AliasCT441';

  <<__LateInit>>
  private AliasCT441 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT441 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT441 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT441>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT441>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT441 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT441> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT442 = void|KeyedTraversable<arraykey, mixed>;
              type AliasCT442 = CT442;

  
class CheckAliasCT442<T as AliasCT442> extends BaseCheck {
  const type T = AliasCT442;
  const string NAME = 'AliasCT442';

  <<__LateInit>>
  private AliasCT442 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT442 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT442 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT442>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT442>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT442 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT442> {
    return vec[dict[],keyset[],null,vec[]];
  }
}
case type CT443 = MyEnum|MyEnum;
              type AliasCT443 = CT443;

  
class CheckAliasCT443<T as AliasCT443> extends BaseCheck {
  const type T = AliasCT443;
  const string NAME = 'AliasCT443';

  <<__LateInit>>
  private AliasCT443 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT443 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT443 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT443>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT443>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT443 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT443> {
    return vec['B',MyEnum::A];
  }
}
case type CT444 = arraykey|MyEnum;
              type AliasCT444 = CT444;

  
class CheckAliasCT444<T as AliasCT444> extends BaseCheck {
  const type T = AliasCT444;
  const string NAME = 'AliasCT444';

  <<__LateInit>>
  private AliasCT444 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT444 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT444 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT444>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT444>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT444 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT444> {
    return vec['','B','hello world',0,1,MyEnum::A];
  }
}
case type CT445 = dynamic|MyEnum;
              type AliasCT445 = CT445;

  
class CheckAliasCT445<T as AliasCT445> extends BaseCheck {
  const type T = AliasCT445;
  const string NAME = 'AliasCT445';

  <<__LateInit>>
  private AliasCT445 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT445 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT445 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT445>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT445>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT445 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT445> {
    return vec['B',MyEnum::A,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT446 = int|MyEnum;
              type AliasCT446 = CT446;

  
class CheckAliasCT446<T as AliasCT446> extends BaseCheck {
  const type T = AliasCT446;
  const string NAME = 'AliasCT446';

  <<__LateInit>>
  private AliasCT446 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT446 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT446 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT446>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT446>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT446 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT446> {
    return vec['B',0,1,MyEnum::A];
  }
}
case type CT447 = mixed|MyEnum;
              type AliasCT447 = CT447;

  
class CheckAliasCT447<T as AliasCT447> extends BaseCheck {
  const type T = AliasCT447;
  const string NAME = 'AliasCT447';

  <<__LateInit>>
  private AliasCT447 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT447 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT447 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT447>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT447>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT447 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT447> {
    return vec['','B','hello world',0,1,MyEnum::A,false,null,true];
  }
}
case type CT448 = nonnull|MyEnum;
              type AliasCT448 = CT448;

  
class CheckAliasCT448<T as AliasCT448> extends BaseCheck {
  const type T = AliasCT448;
  const string NAME = 'AliasCT448';

  <<__LateInit>>
  private AliasCT448 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT448 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT448 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT448>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT448>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT448 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT448> {
    return vec['','B','hello world',0,1,MyEnum::A,false,true];
  }
}
case type CT449 = noreturn|MyEnum;
              type AliasCT449 = CT449;

  
class CheckAliasCT449<T as AliasCT449> extends BaseCheck {
  const type T = AliasCT449;
  const string NAME = 'AliasCT449';

  <<__LateInit>>
  private AliasCT449 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT449 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT449 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT449>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT449>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT449 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT449> {
    return vec['B',MyEnum::A];
  }
}
case type CT450 = nothing|MyEnum;
              type AliasCT450 = CT450;

  
class CheckAliasCT450<T as AliasCT450> extends BaseCheck {
  const type T = AliasCT450;
  const string NAME = 'AliasCT450';

  <<__LateInit>>
  private AliasCT450 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT450 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT450 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT450>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT450>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT450 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT450> {
    return vec['B',MyEnum::A];
  }
}
case type CT451 = null|MyEnum;
              type AliasCT451 = CT451;

  
class CheckAliasCT451<T as AliasCT451> extends BaseCheck {
  const type T = AliasCT451;
  const string NAME = 'AliasCT451';

  <<__LateInit>>
  private AliasCT451 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT451 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT451 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT451>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT451>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT451 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT451> {
    return vec['B',MyEnum::A,null];
  }
}
case type CT452 = string|MyEnum;
              type AliasCT452 = CT452;

  
class CheckAliasCT452<T as AliasCT452> extends BaseCheck {
  const type T = AliasCT452;
  const string NAME = 'AliasCT452';

  <<__LateInit>>
  private AliasCT452 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT452 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT452 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT452>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT452>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT452 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT452> {
    return vec['','B','hello world',MyEnum::A];
  }
}
case type CT453 = void|MyEnum;
              type AliasCT453 = CT453;

  
class CheckAliasCT453<T as AliasCT453> extends BaseCheck {
  const type T = AliasCT453;
  const string NAME = 'AliasCT453';

  <<__LateInit>>
  private AliasCT453 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT453 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT453 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT453>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT453>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT453 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT453> {
    return vec['B',MyEnum::A,null];
  }
}
case type CT454 = MyTrait|MyTrait;
              type AliasCT454 = CT454;

  
class CheckAliasCT454<T as AliasCT454> extends BaseCheck {
  const type T = AliasCT454;
  const string NAME = 'AliasCT454';

  <<__LateInit>>
  private AliasCT454 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT454 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT454 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT454>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT454>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT454 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT454> {
    return vec[];
  }
}
case type CT455 = ReifiedClass<null>|MyTrait;
              type AliasCT455 = CT455;

  
class CheckAliasCT455<T as AliasCT455> extends BaseCheck {
  const type T = AliasCT455;
  const string NAME = 'AliasCT455';

  <<__LateInit>>
  private AliasCT455 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT455 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT455 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT455>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT455>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT455 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT455> {
    return vec[new ReifiedClass<null>()];
  }
}
case type CT456 = Stringish|MyTrait;
              type AliasCT456 = CT456;

  
class CheckAliasCT456<T as AliasCT456> extends BaseCheck {
  const type T = AliasCT456;
  const string NAME = 'AliasCT456';

  <<__LateInit>>
  private AliasCT456 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT456 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT456 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT456>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT456>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT456 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT456> {
    return vec['','hello world',new StringishObj()];
  }
}
case type CT457 = Traversable<mixed>|MyTrait;
              type AliasCT457 = CT457;

  
class CheckAliasCT457<T as AliasCT457> extends BaseCheck {
  const type T = AliasCT457;
  const string NAME = 'AliasCT457';

  <<__LateInit>>
  private AliasCT457 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT457 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT457 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT457>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT457>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT457 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT457> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT458 = XHPChild|MyTrait;
              type AliasCT458 = CT458;

  
class CheckAliasCT458<T as AliasCT458> extends BaseCheck {
  const type T = AliasCT458;
  const string NAME = 'AliasCT458';

  <<__LateInit>>
  private AliasCT458 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT458 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT458 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT458>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT458>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT458 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT458> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT459 = arraykey|MyTrait;
              type AliasCT459 = CT459;

  
class CheckAliasCT459<T as AliasCT459> extends BaseCheck {
  const type T = AliasCT459;
  const string NAME = 'AliasCT459';

  <<__LateInit>>
  private AliasCT459 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT459 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT459 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT459>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT459>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT459 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT459> {
    return vec['','hello world',0,1];
  }
}
case type CT460 = bool|MyTrait;
              type AliasCT460 = CT460;

  
class CheckAliasCT460<T as AliasCT460> extends BaseCheck {
  const type T = AliasCT460;
  const string NAME = 'AliasCT460';

  <<__LateInit>>
  private AliasCT460 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT460 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT460 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT460>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT460>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT460 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT460> {
    return vec[false,true];
  }
}
case type CT461 = dict<arraykey, mixed>|MyTrait;
              type AliasCT461 = CT461;

  
class CheckAliasCT461<T as AliasCT461> extends BaseCheck {
  const type T = AliasCT461;
  const string NAME = 'AliasCT461';

  <<__LateInit>>
  private AliasCT461 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT461 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT461 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT461>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT461>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT461 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT461> {
    return vec[dict[]];
  }
}
case type CT462 = dynamic|MyTrait;
              type AliasCT462 = CT462;

  
class CheckAliasCT462<T as AliasCT462> extends BaseCheck {
  const type T = AliasCT462;
  const string NAME = 'AliasCT462';

  <<__LateInit>>
  private AliasCT462 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT462 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT462 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT462>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT462>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT462 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT462> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT463 = float|MyTrait;
              type AliasCT463 = CT463;

  
class CheckAliasCT463<T as AliasCT463> extends BaseCheck {
  const type T = AliasCT463;
  const string NAME = 'AliasCT463';

  <<__LateInit>>
  private AliasCT463 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT463 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT463 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT463>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT463>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT463 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT463> {
    return vec[0.0,3.14];
  }
}
case type CT464 = int|MyTrait;
              type AliasCT464 = CT464;

  
class CheckAliasCT464<T as AliasCT464> extends BaseCheck {
  const type T = AliasCT464;
  const string NAME = 'AliasCT464';

  <<__LateInit>>
  private AliasCT464 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT464 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT464 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT464>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT464>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT464 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT464> {
    return vec[0,1];
  }
}
case type CT465 = keyset<arraykey>|MyTrait;
              type AliasCT465 = CT465;

  
class CheckAliasCT465<T as AliasCT465> extends BaseCheck {
  const type T = AliasCT465;
  const string NAME = 'AliasCT465';

  <<__LateInit>>
  private AliasCT465 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT465 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT465 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT465>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT465>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT465 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT465> {
    return vec[keyset[]];
  }
}
case type CT466 = mixed|MyTrait;
              type AliasCT466 = CT466;

  
class CheckAliasCT466<T as AliasCT466> extends BaseCheck {
  const type T = AliasCT466;
  const string NAME = 'AliasCT466';

  <<__LateInit>>
  private AliasCT466 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT466 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT466 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT466>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT466>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT466 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT466> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT467 = nonnull|MyTrait;
              type AliasCT467 = CT467;

  
class CheckAliasCT467<T as AliasCT467> extends BaseCheck {
  const type T = AliasCT467;
  const string NAME = 'AliasCT467';

  <<__LateInit>>
  private AliasCT467 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT467 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT467 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT467>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT467>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT467 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT467> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT468 = noreturn|MyTrait;
              type AliasCT468 = CT468;

  
class CheckAliasCT468<T as AliasCT468> extends BaseCheck {
  const type T = AliasCT468;
  const string NAME = 'AliasCT468';

  <<__LateInit>>
  private AliasCT468 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT468 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT468 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT468>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT468>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT468 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT468> {
    return vec[];
  }
}
case type CT469 = nothing|MyTrait;
              type AliasCT469 = CT469;

  
class CheckAliasCT469<T as AliasCT469> extends BaseCheck {
  const type T = AliasCT469;
  const string NAME = 'AliasCT469';

  <<__LateInit>>
  private AliasCT469 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT469 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT469 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT469>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT469>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT469 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT469> {
    return vec[];
  }
}
case type CT470 = null|MyTrait;
              type AliasCT470 = CT470;

  
class CheckAliasCT470<T as AliasCT470> extends BaseCheck {
  const type T = AliasCT470;
  const string NAME = 'AliasCT470';

  <<__LateInit>>
  private AliasCT470 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT470 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT470 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT470>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT470>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT470 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT470> {
    return vec[null];
  }
}
case type CT471 = num|MyTrait;
              type AliasCT471 = CT471;

  
class CheckAliasCT471<T as AliasCT471> extends BaseCheck {
  const type T = AliasCT471;
  const string NAME = 'AliasCT471';

  <<__LateInit>>
  private AliasCT471 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT471 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT471 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT471>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT471>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT471 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT471> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT472 = resource|MyTrait;
              type AliasCT472 = CT472;

  
class CheckAliasCT472<T as AliasCT472> extends BaseCheck {
  const type T = AliasCT472;
  const string NAME = 'AliasCT472';

  <<__LateInit>>
  private AliasCT472 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT472 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT472 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT472>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT472>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT472 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT472> {
    return vec[imagecreate(10, 10)];
  }
}
case type CT473 = shape(...)|MyTrait;
              type AliasCT473 = CT473;

  
class CheckAliasCT473<T as AliasCT473> extends BaseCheck {
  const type T = AliasCT473;
  const string NAME = 'AliasCT473';

  <<__LateInit>>
  private AliasCT473 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT473 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT473 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT473>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT473>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT473 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT473> {
    return vec[shape('x' => 10),shape()];
  }
}
case type CT474 = string|MyTrait;
              type AliasCT474 = CT474;

  
class CheckAliasCT474<T as AliasCT474> extends BaseCheck {
  const type T = AliasCT474;
  const string NAME = 'AliasCT474';

  <<__LateInit>>
  private AliasCT474 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT474 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT474 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT474>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT474>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT474 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT474> {
    return vec['','hello world'];
  }
}
case type CT475 = vec<mixed>|MyTrait;
              type AliasCT475 = CT475;

  
class CheckAliasCT475<T as AliasCT475> extends BaseCheck {
  const type T = AliasCT475;
  const string NAME = 'AliasCT475';

  <<__LateInit>>
  private AliasCT475 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT475 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT475 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT475>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT475>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT475 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT475> {
    return vec[vec[]];
  }
}
case type CT476 = vec_or_dict<string>|MyTrait;
              type AliasCT476 = CT476;

  
class CheckAliasCT476<T as AliasCT476> extends BaseCheck {
  const type T = AliasCT476;
  const string NAME = 'AliasCT476';

  <<__LateInit>>
  private AliasCT476 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT476 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT476 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT476>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT476>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT476 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT476> {
    return vec[dict[],vec[]];
  }
}
case type CT477 = void|MyTrait;
              type AliasCT477 = CT477;

  
class CheckAliasCT477<T as AliasCT477> extends BaseCheck {
  const type T = AliasCT477;
  const string NAME = 'AliasCT477';

  <<__LateInit>>
  private AliasCT477 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT477 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT477 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT477>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT477>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT477 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT477> {
    return vec[null];
  }
}
case type CT478 = ReifiedClass<null>|ReifiedClass<null>;
              type AliasCT478 = CT478;

  
class CheckAliasCT478<T as AliasCT478> extends BaseCheck {
  const type T = AliasCT478;
  const string NAME = 'AliasCT478';

  <<__LateInit>>
  private AliasCT478 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT478 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT478 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT478>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT478>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT478 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT478> {
    return vec[new ReifiedClass<null>()];
  }
}
case type CT479 = Stringish|ReifiedClass<null>;
              type AliasCT479 = CT479;

  
class CheckAliasCT479<T as AliasCT479> extends BaseCheck {
  const type T = AliasCT479;
  const string NAME = 'AliasCT479';

  <<__LateInit>>
  private AliasCT479 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT479 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT479 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT479>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT479>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT479 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT479> {
    return vec['','hello world',new ReifiedClass<null>(),new StringishObj()];
  }
}
case type CT480 = Traversable<mixed>|ReifiedClass<null>;
              type AliasCT480 = CT480;

  
class CheckAliasCT480<T as AliasCT480> extends BaseCheck {
  const type T = AliasCT480;
  const string NAME = 'AliasCT480';

  <<__LateInit>>
  private AliasCT480 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT480 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT480 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT480>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT480>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT480 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT480> {
    return vec[dict[],keyset[],new ReifiedClass<null>(),vec[]];
  }
}
case type CT481 = XHPChild|ReifiedClass<null>;
              type AliasCT481 = CT481;

  
class CheckAliasCT481<T as AliasCT481> extends BaseCheck {
  const type T = AliasCT481;
  const string NAME = 'AliasCT481';

  <<__LateInit>>
  private AliasCT481 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT481 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT481 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT481>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT481>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT481 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT481> {
    return vec['','hello world',0,1,<my-xhp/>,new ReifiedClass<null>()];
  }
}
case type CT482 = arraykey|ReifiedClass<null>;
              type AliasCT482 = CT482;

  
class CheckAliasCT482<T as AliasCT482> extends BaseCheck {
  const type T = AliasCT482;
  const string NAME = 'AliasCT482';

  <<__LateInit>>
  private AliasCT482 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT482 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT482 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT482>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT482>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT482 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT482> {
    return vec['','hello world',0,1,new ReifiedClass<null>()];
  }
}
case type CT483 = bool|ReifiedClass<null>;
              type AliasCT483 = CT483;

  
class CheckAliasCT483<T as AliasCT483> extends BaseCheck {
  const type T = AliasCT483;
  const string NAME = 'AliasCT483';

  <<__LateInit>>
  private AliasCT483 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT483 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT483 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT483>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT483>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT483 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT483> {
    return vec[false,new ReifiedClass<null>(),true];
  }
}
case type CT484 = dict<arraykey, mixed>|ReifiedClass<null>;
              type AliasCT484 = CT484;

  
class CheckAliasCT484<T as AliasCT484> extends BaseCheck {
  const type T = AliasCT484;
  const string NAME = 'AliasCT484';

  <<__LateInit>>
  private AliasCT484 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT484 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT484 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT484>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT484>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT484 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT484> {
    return vec[dict[],new ReifiedClass<null>()];
  }
}
case type CT485 = dynamic|ReifiedClass<null>;
              type AliasCT485 = CT485;

  
class CheckAliasCT485<T as AliasCT485> extends BaseCheck {
  const type T = AliasCT485;
  const string NAME = 'AliasCT485';

  <<__LateInit>>
  private AliasCT485 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT485 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT485 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT485>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT485>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT485 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT485> {
    return vec[false,new ReifiedClass<null>(),null,shape('x' => 10),shape(),true];
  }
}
case type CT486 = float|ReifiedClass<null>;
              type AliasCT486 = CT486;

  
class CheckAliasCT486<T as AliasCT486> extends BaseCheck {
  const type T = AliasCT486;
  const string NAME = 'AliasCT486';

  <<__LateInit>>
  private AliasCT486 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT486 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT486 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT486>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT486>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT486 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT486> {
    return vec[0.0,3.14,new ReifiedClass<null>()];
  }
}
case type CT487 = int|ReifiedClass<null>;
              type AliasCT487 = CT487;

  
class CheckAliasCT487<T as AliasCT487> extends BaseCheck {
  const type T = AliasCT487;
  const string NAME = 'AliasCT487';

  <<__LateInit>>
  private AliasCT487 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT487 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT487 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT487>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT487>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT487 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT487> {
    return vec[0,1,new ReifiedClass<null>()];
  }
}
case type CT488 = keyset<arraykey>|ReifiedClass<null>;
              type AliasCT488 = CT488;

  
class CheckAliasCT488<T as AliasCT488> extends BaseCheck {
  const type T = AliasCT488;
  const string NAME = 'AliasCT488';

  <<__LateInit>>
  private AliasCT488 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT488 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT488 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT488>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT488>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT488 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT488> {
    return vec[keyset[],new ReifiedClass<null>()];
  }
}
case type CT489 = mixed|ReifiedClass<null>;
              type AliasCT489 = CT489;

  
class CheckAliasCT489<T as AliasCT489> extends BaseCheck {
  const type T = AliasCT489;
  const string NAME = 'AliasCT489';

  <<__LateInit>>
  private AliasCT489 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT489 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT489 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT489>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT489>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT489 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT489> {
    return vec['','hello world',0,1,false,new ReifiedClass<null>(),null,true];
  }
}
case type CT490 = nonnull|ReifiedClass<null>;
              type AliasCT490 = CT490;

  
class CheckAliasCT490<T as AliasCT490> extends BaseCheck {
  const type T = AliasCT490;
  const string NAME = 'AliasCT490';

  <<__LateInit>>
  private AliasCT490 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT490 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT490 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT490>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT490>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT490 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT490> {
    return vec['','hello world',0,1,false,new ReifiedClass<null>(),true];
  }
}
case type CT491 = noreturn|ReifiedClass<null>;
              type AliasCT491 = CT491;

  
class CheckAliasCT491<T as AliasCT491> extends BaseCheck {
  const type T = AliasCT491;
  const string NAME = 'AliasCT491';

  <<__LateInit>>
  private AliasCT491 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT491 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT491 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT491>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT491>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT491 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT491> {
    return vec[new ReifiedClass<null>()];
  }
}
case type CT492 = nothing|ReifiedClass<null>;
              type AliasCT492 = CT492;

  
class CheckAliasCT492<T as AliasCT492> extends BaseCheck {
  const type T = AliasCT492;
  const string NAME = 'AliasCT492';

  <<__LateInit>>
  private AliasCT492 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT492 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT492 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT492>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT492>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT492 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT492> {
    return vec[new ReifiedClass<null>()];
  }
}
case type CT493 = null|ReifiedClass<null>;
              type AliasCT493 = CT493;

  
class CheckAliasCT493<T as AliasCT493> extends BaseCheck {
  const type T = AliasCT493;
  const string NAME = 'AliasCT493';

  <<__LateInit>>
  private AliasCT493 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT493 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT493 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT493>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT493>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT493 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT493> {
    return vec[new ReifiedClass<null>(),null];
  }
}
case type CT494 = num|ReifiedClass<null>;
              type AliasCT494 = CT494;

  
class CheckAliasCT494<T as AliasCT494> extends BaseCheck {
  const type T = AliasCT494;
  const string NAME = 'AliasCT494';

  <<__LateInit>>
  private AliasCT494 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT494 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT494 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT494>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT494>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT494 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT494> {
    return vec[0,0.0,1,3.14,new ReifiedClass<null>()];
  }
}
case type CT495 = resource|ReifiedClass<null>;
              type AliasCT495 = CT495;

  
class CheckAliasCT495<T as AliasCT495> extends BaseCheck {
  const type T = AliasCT495;
  const string NAME = 'AliasCT495';

  <<__LateInit>>
  private AliasCT495 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT495 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT495 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT495>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT495>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT495 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT495> {
    return vec[imagecreate(10, 10),new ReifiedClass<null>()];
  }
}
case type CT496 = shape(...)|ReifiedClass<null>;
              type AliasCT496 = CT496;

  
class CheckAliasCT496<T as AliasCT496> extends BaseCheck {
  const type T = AliasCT496;
  const string NAME = 'AliasCT496';

  <<__LateInit>>
  private AliasCT496 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT496 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT496 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT496>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT496>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT496 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT496> {
    return vec[new ReifiedClass<null>(),shape('x' => 10),shape()];
  }
}
case type CT497 = string|ReifiedClass<null>;
              type AliasCT497 = CT497;

  
class CheckAliasCT497<T as AliasCT497> extends BaseCheck {
  const type T = AliasCT497;
  const string NAME = 'AliasCT497';

  <<__LateInit>>
  private AliasCT497 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT497 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT497 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT497>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT497>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT497 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT497> {
    return vec['','hello world',new ReifiedClass<null>()];
  }
}
case type CT498 = vec<mixed>|ReifiedClass<null>;
              type AliasCT498 = CT498;

  
class CheckAliasCT498<T as AliasCT498> extends BaseCheck {
  const type T = AliasCT498;
  const string NAME = 'AliasCT498';

  <<__LateInit>>
  private AliasCT498 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT498 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT498 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT498>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT498>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT498 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT498> {
    return vec[new ReifiedClass<null>(),vec[]];
  }
}
case type CT499 = vec_or_dict<string>|ReifiedClass<null>;
              type AliasCT499 = CT499;

  
class CheckAliasCT499<T as AliasCT499> extends BaseCheck {
  const type T = AliasCT499;
  const string NAME = 'AliasCT499';

  <<__LateInit>>
  private AliasCT499 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT499 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT499 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT499>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT499>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT499 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT499> {
    return vec[dict[],new ReifiedClass<null>(),vec[]];
  }
}
case type CT500 = void|ReifiedClass<null>;
              type AliasCT500 = CT500;

  
class CheckAliasCT500<T as AliasCT500> extends BaseCheck {
  const type T = AliasCT500;
  const string NAME = 'AliasCT500';

  <<__LateInit>>
  private AliasCT500 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT500 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT500 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT500>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT500>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT500 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT500> {
    return vec[new ReifiedClass<null>(),null];
  }
}
case type CT501 = Stringish|Stringish;
              type AliasCT501 = CT501;

  
class CheckAliasCT501<T as AliasCT501> extends BaseCheck {
  const type T = AliasCT501;
  const string NAME = 'AliasCT501';

  <<__LateInit>>
  private AliasCT501 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT501 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT501 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT501>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT501>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT501 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT501> {
    return vec['','hello world',new StringishObj()];
  }
}
case type CT502 = Traversable<mixed>|Stringish;
              type AliasCT502 = CT502;

  
class CheckAliasCT502<T as AliasCT502> extends BaseCheck {
  const type T = AliasCT502;
  const string NAME = 'AliasCT502';

  <<__LateInit>>
  private AliasCT502 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT502 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT502 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT502>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT502>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT502 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT502> {
    return vec['','hello world',dict[],keyset[],new StringishObj(),vec[]];
  }
}
case type CT503 = XHPChild|Stringish;
              type AliasCT503 = CT503;

  
class CheckAliasCT503<T as AliasCT503> extends BaseCheck {
  const type T = AliasCT503;
  const string NAME = 'AliasCT503';

  <<__LateInit>>
  private AliasCT503 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT503 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT503 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT503>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT503>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT503 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT503> {
    return vec['','hello world',0,1,<my-xhp/>,new StringishObj()];
  }
}
case type CT504 = arraykey|Stringish;
              type AliasCT504 = CT504;

  
class CheckAliasCT504<T as AliasCT504> extends BaseCheck {
  const type T = AliasCT504;
  const string NAME = 'AliasCT504';

  <<__LateInit>>
  private AliasCT504 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT504 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT504 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT504>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT504>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT504 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT504> {
    return vec['','hello world',0,1,new StringishObj()];
  }
}
case type CT505 = bool|Stringish;
              type AliasCT505 = CT505;

  
class CheckAliasCT505<T as AliasCT505> extends BaseCheck {
  const type T = AliasCT505;
  const string NAME = 'AliasCT505';

  <<__LateInit>>
  private AliasCT505 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT505 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT505 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT505>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT505>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT505 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT505> {
    return vec['','hello world',false,new StringishObj(),true];
  }
}
case type CT506 = dict<arraykey, mixed>|Stringish;
              type AliasCT506 = CT506;

  
class CheckAliasCT506<T as AliasCT506> extends BaseCheck {
  const type T = AliasCT506;
  const string NAME = 'AliasCT506';

  <<__LateInit>>
  private AliasCT506 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT506 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT506 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT506>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT506>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT506 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT506> {
    return vec['','hello world',dict[],new StringishObj()];
  }
}
case type CT507 = dynamic|Stringish;
              type AliasCT507 = CT507;

  
class CheckAliasCT507<T as AliasCT507> extends BaseCheck {
  const type T = AliasCT507;
  const string NAME = 'AliasCT507';

  <<__LateInit>>
  private AliasCT507 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT507 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT507 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT507>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT507>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT507 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT507> {
    return vec['','hello world',false,new StringishObj(),null,shape('x' => 10),shape(),true];
  }
}
case type CT508 = float|Stringish;
              type AliasCT508 = CT508;

  
class CheckAliasCT508<T as AliasCT508> extends BaseCheck {
  const type T = AliasCT508;
  const string NAME = 'AliasCT508';

  <<__LateInit>>
  private AliasCT508 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT508 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT508 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT508>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT508>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT508 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT508> {
    return vec['','hello world',0.0,3.14,new StringishObj()];
  }
}
case type CT509 = int|Stringish;
              type AliasCT509 = CT509;

  
class CheckAliasCT509<T as AliasCT509> extends BaseCheck {
  const type T = AliasCT509;
  const string NAME = 'AliasCT509';

  <<__LateInit>>
  private AliasCT509 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT509 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT509 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT509>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT509>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT509 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT509> {
    return vec['','hello world',0,1,new StringishObj()];
  }
}
case type CT510 = keyset<arraykey>|Stringish;
              type AliasCT510 = CT510;

  
class CheckAliasCT510<T as AliasCT510> extends BaseCheck {
  const type T = AliasCT510;
  const string NAME = 'AliasCT510';

  <<__LateInit>>
  private AliasCT510 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT510 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT510 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT510>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT510>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT510 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT510> {
    return vec['','hello world',keyset[],new StringishObj()];
  }
}
case type CT511 = mixed|Stringish;
              type AliasCT511 = CT511;

  
class CheckAliasCT511<T as AliasCT511> extends BaseCheck {
  const type T = AliasCT511;
  const string NAME = 'AliasCT511';

  <<__LateInit>>
  private AliasCT511 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT511 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT511 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT511>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT511>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT511 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT511> {
    return vec['','hello world',0,1,false,new StringishObj(),null,true];
  }
}
case type CT512 = nonnull|Stringish;
              type AliasCT512 = CT512;

  
class CheckAliasCT512<T as AliasCT512> extends BaseCheck {
  const type T = AliasCT512;
  const string NAME = 'AliasCT512';

  <<__LateInit>>
  private AliasCT512 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT512 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT512 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT512>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT512>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT512 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT512> {
    return vec['','hello world',0,1,false,new StringishObj(),true];
  }
}
case type CT513 = noreturn|Stringish;
              type AliasCT513 = CT513;

  
class CheckAliasCT513<T as AliasCT513> extends BaseCheck {
  const type T = AliasCT513;
  const string NAME = 'AliasCT513';

  <<__LateInit>>
  private AliasCT513 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT513 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT513 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT513>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT513>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT513 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT513> {
    return vec['','hello world',new StringishObj()];
  }
}
case type CT514 = nothing|Stringish;
              type AliasCT514 = CT514;

  
class CheckAliasCT514<T as AliasCT514> extends BaseCheck {
  const type T = AliasCT514;
  const string NAME = 'AliasCT514';

  <<__LateInit>>
  private AliasCT514 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT514 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT514 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT514>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT514>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT514 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT514> {
    return vec['','hello world',new StringishObj()];
  }
}
case type CT515 = null|Stringish;
              type AliasCT515 = CT515;

  
class CheckAliasCT515<T as AliasCT515> extends BaseCheck {
  const type T = AliasCT515;
  const string NAME = 'AliasCT515';

  <<__LateInit>>
  private AliasCT515 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT515 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT515 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT515>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT515>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT515 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT515> {
    return vec['','hello world',new StringishObj(),null];
  }
}
case type CT516 = num|Stringish;
              type AliasCT516 = CT516;

  
class CheckAliasCT516<T as AliasCT516> extends BaseCheck {
  const type T = AliasCT516;
  const string NAME = 'AliasCT516';

  <<__LateInit>>
  private AliasCT516 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT516 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT516 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT516>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT516>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT516 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT516> {
    return vec['','hello world',0,0.0,1,3.14,new StringishObj()];
  }
}
case type CT517 = resource|Stringish;
              type AliasCT517 = CT517;

  
class CheckAliasCT517<T as AliasCT517> extends BaseCheck {
  const type T = AliasCT517;
  const string NAME = 'AliasCT517';

  <<__LateInit>>
  private AliasCT517 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT517 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT517 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT517>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT517>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT517 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT517> {
    return vec['','hello world',imagecreate(10, 10),new StringishObj()];
  }
}
case type CT518 = shape(...)|Stringish;
              type AliasCT518 = CT518;

  
class CheckAliasCT518<T as AliasCT518> extends BaseCheck {
  const type T = AliasCT518;
  const string NAME = 'AliasCT518';

  <<__LateInit>>
  private AliasCT518 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT518 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT518 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT518>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT518>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT518 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT518> {
    return vec['','hello world',new StringishObj(),shape('x' => 10),shape()];
  }
}
case type CT519 = string|Stringish;
              type AliasCT519 = CT519;

  
class CheckAliasCT519<T as AliasCT519> extends BaseCheck {
  const type T = AliasCT519;
  const string NAME = 'AliasCT519';

  <<__LateInit>>
  private AliasCT519 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT519 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT519 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT519>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT519>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT519 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT519> {
    return vec['','hello world',new StringishObj()];
  }
}
case type CT520 = vec<mixed>|Stringish;
              type AliasCT520 = CT520;

  
class CheckAliasCT520<T as AliasCT520> extends BaseCheck {
  const type T = AliasCT520;
  const string NAME = 'AliasCT520';

  <<__LateInit>>
  private AliasCT520 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT520 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT520 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT520>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT520>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT520 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT520> {
    return vec['','hello world',new StringishObj(),vec[]];
  }
}
case type CT521 = vec_or_dict<string>|Stringish;
              type AliasCT521 = CT521;

  
class CheckAliasCT521<T as AliasCT521> extends BaseCheck {
  const type T = AliasCT521;
  const string NAME = 'AliasCT521';

  <<__LateInit>>
  private AliasCT521 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT521 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT521 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT521>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT521>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT521 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT521> {
    return vec['','hello world',dict[],new StringishObj(),vec[]];
  }
}
case type CT522 = void|Stringish;
              type AliasCT522 = CT522;

  
class CheckAliasCT522<T as AliasCT522> extends BaseCheck {
  const type T = AliasCT522;
  const string NAME = 'AliasCT522';

  <<__LateInit>>
  private AliasCT522 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT522 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT522 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT522>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT522>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT522 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT522> {
    return vec['','hello world',new StringishObj(),null];
  }
}
case type CT523 = Traversable<mixed>|Traversable<mixed>;
              type AliasCT523 = CT523;

  
class CheckAliasCT523<T as AliasCT523> extends BaseCheck {
  const type T = AliasCT523;
  const string NAME = 'AliasCT523';

  <<__LateInit>>
  private AliasCT523 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT523 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT523 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT523>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT523>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT523 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT523> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT524 = XHPChild|Traversable<mixed>;
              type AliasCT524 = CT524;

  
class CheckAliasCT524<T as AliasCT524> extends BaseCheck {
  const type T = AliasCT524;
  const string NAME = 'AliasCT524';

  <<__LateInit>>
  private AliasCT524 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT524 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT524 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT524>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT524>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT524 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT524> {
    return vec['','hello world',0,1,<my-xhp/>,dict[],keyset[],vec[]];
  }
}
case type CT525 = arraykey|Traversable<mixed>;
              type AliasCT525 = CT525;

  
class CheckAliasCT525<T as AliasCT525> extends BaseCheck {
  const type T = AliasCT525;
  const string NAME = 'AliasCT525';

  <<__LateInit>>
  private AliasCT525 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT525 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT525 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT525>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT525>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT525 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT525> {
    return vec['','hello world',0,1,dict[],keyset[],vec[]];
  }
}
case type CT526 = bool|Traversable<mixed>;
              type AliasCT526 = CT526;

  
class CheckAliasCT526<T as AliasCT526> extends BaseCheck {
  const type T = AliasCT526;
  const string NAME = 'AliasCT526';

  <<__LateInit>>
  private AliasCT526 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT526 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT526 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT526>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT526>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT526 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT526> {
    return vec[dict[],false,keyset[],true,vec[]];
  }
}
case type CT527 = dict<arraykey, mixed>|Traversable<mixed>;
              type AliasCT527 = CT527;

  
class CheckAliasCT527<T as AliasCT527> extends BaseCheck {
  const type T = AliasCT527;
  const string NAME = 'AliasCT527';

  <<__LateInit>>
  private AliasCT527 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT527 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT527 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT527>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT527>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT527 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT527> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT528 = dynamic|Traversable<mixed>;
              type AliasCT528 = CT528;

  
class CheckAliasCT528<T as AliasCT528> extends BaseCheck {
  const type T = AliasCT528;
  const string NAME = 'AliasCT528';

  <<__LateInit>>
  private AliasCT528 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT528 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT528 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT528>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT528>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT528 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT528> {
    return vec[dict[],false,keyset[],null,shape('x' => 10),shape(),true,vec[]];
  }
}
case type CT529 = float|Traversable<mixed>;
              type AliasCT529 = CT529;

  
class CheckAliasCT529<T as AliasCT529> extends BaseCheck {
  const type T = AliasCT529;
  const string NAME = 'AliasCT529';

  <<__LateInit>>
  private AliasCT529 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT529 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT529 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT529>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT529>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT529 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT529> {
    return vec[0.0,3.14,dict[],keyset[],vec[]];
  }
}
case type CT530 = int|Traversable<mixed>;
              type AliasCT530 = CT530;

  
class CheckAliasCT530<T as AliasCT530> extends BaseCheck {
  const type T = AliasCT530;
  const string NAME = 'AliasCT530';

  <<__LateInit>>
  private AliasCT530 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT530 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT530 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT530>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT530>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT530 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT530> {
    return vec[0,1,dict[],keyset[],vec[]];
  }
}
case type CT531 = keyset<arraykey>|Traversable<mixed>;
              type AliasCT531 = CT531;

  
class CheckAliasCT531<T as AliasCT531> extends BaseCheck {
  const type T = AliasCT531;
  const string NAME = 'AliasCT531';

  <<__LateInit>>
  private AliasCT531 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT531 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT531 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT531>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT531>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT531 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT531> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT532 = mixed|Traversable<mixed>;
              type AliasCT532 = CT532;

  
class CheckAliasCT532<T as AliasCT532> extends BaseCheck {
  const type T = AliasCT532;
  const string NAME = 'AliasCT532';

  <<__LateInit>>
  private AliasCT532 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT532 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT532 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT532>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT532>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT532 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT532> {
    return vec['','hello world',0,1,dict[],false,keyset[],null,true,vec[]];
  }
}
case type CT533 = nonnull|Traversable<mixed>;
              type AliasCT533 = CT533;

  
class CheckAliasCT533<T as AliasCT533> extends BaseCheck {
  const type T = AliasCT533;
  const string NAME = 'AliasCT533';

  <<__LateInit>>
  private AliasCT533 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT533 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT533 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT533>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT533>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT533 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT533> {
    return vec['','hello world',0,1,dict[],false,keyset[],true,vec[]];
  }
}
case type CT534 = noreturn|Traversable<mixed>;
              type AliasCT534 = CT534;

  
class CheckAliasCT534<T as AliasCT534> extends BaseCheck {
  const type T = AliasCT534;
  const string NAME = 'AliasCT534';

  <<__LateInit>>
  private AliasCT534 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT534 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT534 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT534>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT534>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT534 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT534> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT535 = nothing|Traversable<mixed>;
              type AliasCT535 = CT535;

  
class CheckAliasCT535<T as AliasCT535> extends BaseCheck {
  const type T = AliasCT535;
  const string NAME = 'AliasCT535';

  <<__LateInit>>
  private AliasCT535 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT535 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT535 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT535>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT535>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT535 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT535> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT536 = null|Traversable<mixed>;
              type AliasCT536 = CT536;

  
class CheckAliasCT536<T as AliasCT536> extends BaseCheck {
  const type T = AliasCT536;
  const string NAME = 'AliasCT536';

  <<__LateInit>>
  private AliasCT536 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT536 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT536 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT536>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT536>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT536 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT536> {
    return vec[dict[],keyset[],null,vec[]];
  }
}
case type CT537 = num|Traversable<mixed>;
              type AliasCT537 = CT537;

  
class CheckAliasCT537<T as AliasCT537> extends BaseCheck {
  const type T = AliasCT537;
  const string NAME = 'AliasCT537';

  <<__LateInit>>
  private AliasCT537 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT537 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT537 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT537>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT537>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT537 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT537> {
    return vec[0,0.0,1,3.14,dict[],keyset[],vec[]];
  }
}
case type CT538 = resource|Traversable<mixed>;
              type AliasCT538 = CT538;

  
class CheckAliasCT538<T as AliasCT538> extends BaseCheck {
  const type T = AliasCT538;
  const string NAME = 'AliasCT538';

  <<__LateInit>>
  private AliasCT538 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT538 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT538 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT538>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT538>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT538 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT538> {
    return vec[dict[],imagecreate(10, 10),keyset[],vec[]];
  }
}
case type CT539 = shape(...)|Traversable<mixed>;
              type AliasCT539 = CT539;

  
class CheckAliasCT539<T as AliasCT539> extends BaseCheck {
  const type T = AliasCT539;
  const string NAME = 'AliasCT539';

  <<__LateInit>>
  private AliasCT539 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT539 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT539 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT539>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT539>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT539 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT539> {
    return vec[dict[],keyset[],shape('x' => 10),shape(),vec[]];
  }
}
case type CT540 = string|Traversable<mixed>;
              type AliasCT540 = CT540;

  
class CheckAliasCT540<T as AliasCT540> extends BaseCheck {
  const type T = AliasCT540;
  const string NAME = 'AliasCT540';

  <<__LateInit>>
  private AliasCT540 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT540 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT540 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT540>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT540>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT540 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT540> {
    return vec['','hello world',dict[],keyset[],vec[]];
  }
}
case type CT541 = vec<mixed>|Traversable<mixed>;
              type AliasCT541 = CT541;

  
class CheckAliasCT541<T as AliasCT541> extends BaseCheck {
  const type T = AliasCT541;
  const string NAME = 'AliasCT541';

  <<__LateInit>>
  private AliasCT541 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT541 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT541 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT541>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT541>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT541 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT541> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT542 = vec_or_dict<string>|Traversable<mixed>;
              type AliasCT542 = CT542;

  
class CheckAliasCT542<T as AliasCT542> extends BaseCheck {
  const type T = AliasCT542;
  const string NAME = 'AliasCT542';

  <<__LateInit>>
  private AliasCT542 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT542 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT542 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT542>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT542>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT542 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT542> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT543 = void|Traversable<mixed>;
              type AliasCT543 = CT543;

  
class CheckAliasCT543<T as AliasCT543> extends BaseCheck {
  const type T = AliasCT543;
  const string NAME = 'AliasCT543';

  <<__LateInit>>
  private AliasCT543 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT543 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT543 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT543>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT543>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT543 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT543> {
    return vec[dict[],keyset[],null,vec[]];
  }
}
case type CT544 = XHPChild|XHPChild;
              type AliasCT544 = CT544;

  
class CheckAliasCT544<T as AliasCT544> extends BaseCheck {
  const type T = AliasCT544;
  const string NAME = 'AliasCT544';

  <<__LateInit>>
  private AliasCT544 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT544 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT544 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT544>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT544>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT544 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT544> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT545 = arraykey|XHPChild;
              type AliasCT545 = CT545;

  
class CheckAliasCT545<T as AliasCT545> extends BaseCheck {
  const type T = AliasCT545;
  const string NAME = 'AliasCT545';

  <<__LateInit>>
  private AliasCT545 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT545 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT545 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT545>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT545>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT545 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT545> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT546 = bool|XHPChild;
              type AliasCT546 = CT546;

  
class CheckAliasCT546<T as AliasCT546> extends BaseCheck {
  const type T = AliasCT546;
  const string NAME = 'AliasCT546';

  <<__LateInit>>
  private AliasCT546 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT546 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT546 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT546>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT546>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT546 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT546> {
    return vec['','hello world',0,1,<my-xhp/>,false,true];
  }
}
case type CT547 = dict<arraykey, mixed>|XHPChild;
              type AliasCT547 = CT547;

  
class CheckAliasCT547<T as AliasCT547> extends BaseCheck {
  const type T = AliasCT547;
  const string NAME = 'AliasCT547';

  <<__LateInit>>
  private AliasCT547 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT547 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT547 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT547>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT547>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT547 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT547> {
    return vec['','hello world',0,1,<my-xhp/>,dict[]];
  }
}
case type CT548 = dynamic|XHPChild;
              type AliasCT548 = CT548;

  
class CheckAliasCT548<T as AliasCT548> extends BaseCheck {
  const type T = AliasCT548;
  const string NAME = 'AliasCT548';

  <<__LateInit>>
  private AliasCT548 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT548 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT548 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT548>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT548>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT548 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT548> {
    return vec['','hello world',0,1,<my-xhp/>,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT549 = float|XHPChild;
              type AliasCT549 = CT549;

  
class CheckAliasCT549<T as AliasCT549> extends BaseCheck {
  const type T = AliasCT549;
  const string NAME = 'AliasCT549';

  <<__LateInit>>
  private AliasCT549 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT549 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT549 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT549>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT549>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT549 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT549> {
    return vec['','hello world',0,0.0,1,3.14,<my-xhp/>];
  }
}
case type CT550 = int|XHPChild;
              type AliasCT550 = CT550;

  
class CheckAliasCT550<T as AliasCT550> extends BaseCheck {
  const type T = AliasCT550;
  const string NAME = 'AliasCT550';

  <<__LateInit>>
  private AliasCT550 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT550 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT550 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT550>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT550>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT550 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT550> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT551 = keyset<arraykey>|XHPChild;
              type AliasCT551 = CT551;

  
class CheckAliasCT551<T as AliasCT551> extends BaseCheck {
  const type T = AliasCT551;
  const string NAME = 'AliasCT551';

  <<__LateInit>>
  private AliasCT551 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT551 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT551 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT551>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT551>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT551 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT551> {
    return vec['','hello world',0,1,<my-xhp/>,keyset[]];
  }
}
case type CT552 = mixed|XHPChild;
              type AliasCT552 = CT552;

  
class CheckAliasCT552<T as AliasCT552> extends BaseCheck {
  const type T = AliasCT552;
  const string NAME = 'AliasCT552';

  <<__LateInit>>
  private AliasCT552 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT552 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT552 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT552>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT552>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT552 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT552> {
    return vec['','hello world',0,1,<my-xhp/>,false,null,true];
  }
}
case type CT553 = nonnull|XHPChild;
              type AliasCT553 = CT553;

  
class CheckAliasCT553<T as AliasCT553> extends BaseCheck {
  const type T = AliasCT553;
  const string NAME = 'AliasCT553';

  <<__LateInit>>
  private AliasCT553 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT553 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT553 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT553>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT553>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT553 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT553> {
    return vec['','hello world',0,1,<my-xhp/>,false,true];
  }
}
case type CT554 = noreturn|XHPChild;
              type AliasCT554 = CT554;

  
class CheckAliasCT554<T as AliasCT554> extends BaseCheck {
  const type T = AliasCT554;
  const string NAME = 'AliasCT554';

  <<__LateInit>>
  private AliasCT554 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT554 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT554 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT554>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT554>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT554 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT554> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT555 = nothing|XHPChild;
              type AliasCT555 = CT555;

  
class CheckAliasCT555<T as AliasCT555> extends BaseCheck {
  const type T = AliasCT555;
  const string NAME = 'AliasCT555';

  <<__LateInit>>
  private AliasCT555 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT555 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT555 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT555>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT555>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT555 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT555> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT556 = null|XHPChild;
              type AliasCT556 = CT556;

  
class CheckAliasCT556<T as AliasCT556> extends BaseCheck {
  const type T = AliasCT556;
  const string NAME = 'AliasCT556';

  <<__LateInit>>
  private AliasCT556 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT556 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT556 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT556>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT556>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT556 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT556> {
    return vec['','hello world',0,1,<my-xhp/>,null];
  }
}
case type CT557 = num|XHPChild;
              type AliasCT557 = CT557;

  
class CheckAliasCT557<T as AliasCT557> extends BaseCheck {
  const type T = AliasCT557;
  const string NAME = 'AliasCT557';

  <<__LateInit>>
  private AliasCT557 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT557 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT557 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT557>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT557>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT557 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT557> {
    return vec['','hello world',0,0.0,1,3.14,<my-xhp/>];
  }
}
case type CT558 = resource|XHPChild;
              type AliasCT558 = CT558;

  
class CheckAliasCT558<T as AliasCT558> extends BaseCheck {
  const type T = AliasCT558;
  const string NAME = 'AliasCT558';

  <<__LateInit>>
  private AliasCT558 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT558 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT558 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT558>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT558>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT558 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT558> {
    return vec['','hello world',0,1,<my-xhp/>,imagecreate(10, 10)];
  }
}
case type CT559 = shape(...)|XHPChild;
              type AliasCT559 = CT559;

  
class CheckAliasCT559<T as AliasCT559> extends BaseCheck {
  const type T = AliasCT559;
  const string NAME = 'AliasCT559';

  <<__LateInit>>
  private AliasCT559 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT559 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT559 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT559>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT559>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT559 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT559> {
    return vec['','hello world',0,1,<my-xhp/>,shape('x' => 10),shape()];
  }
}
case type CT560 = string|XHPChild;
              type AliasCT560 = CT560;

  
class CheckAliasCT560<T as AliasCT560> extends BaseCheck {
  const type T = AliasCT560;
  const string NAME = 'AliasCT560';

  <<__LateInit>>
  private AliasCT560 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT560 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT560 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT560>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT560>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT560 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT560> {
    return vec['','hello world',0,1,<my-xhp/>];
  }
}
case type CT561 = vec<mixed>|XHPChild;
              type AliasCT561 = CT561;

  
class CheckAliasCT561<T as AliasCT561> extends BaseCheck {
  const type T = AliasCT561;
  const string NAME = 'AliasCT561';

  <<__LateInit>>
  private AliasCT561 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT561 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT561 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT561>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT561>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT561 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT561> {
    return vec['','hello world',0,1,<my-xhp/>,vec[]];
  }
}
case type CT562 = vec_or_dict<string>|XHPChild;
              type AliasCT562 = CT562;

  
class CheckAliasCT562<T as AliasCT562> extends BaseCheck {
  const type T = AliasCT562;
  const string NAME = 'AliasCT562';

  <<__LateInit>>
  private AliasCT562 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT562 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT562 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT562>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT562>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT562 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT562> {
    return vec['','hello world',0,1,<my-xhp/>,dict[],vec[]];
  }
}
case type CT563 = void|XHPChild;
              type AliasCT563 = CT563;

  
class CheckAliasCT563<T as AliasCT563> extends BaseCheck {
  const type T = AliasCT563;
  const string NAME = 'AliasCT563';

  <<__LateInit>>
  private AliasCT563 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT563 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT563 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT563>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT563>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT563 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT563> {
    return vec['','hello world',0,1,<my-xhp/>,null];
  }
}
case type CT564 = arraykey|arraykey;
              type AliasCT564 = CT564;

  
class CheckAliasCT564<T as AliasCT564> extends BaseCheck {
  const type T = AliasCT564;
  const string NAME = 'AliasCT564';

  <<__LateInit>>
  private AliasCT564 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT564 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT564 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT564>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT564>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT564 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT564> {
    return vec['','hello world',0,1];
  }
}
case type CT565 = bool|arraykey;
              type AliasCT565 = CT565;

  
class CheckAliasCT565<T as AliasCT565> extends BaseCheck {
  const type T = AliasCT565;
  const string NAME = 'AliasCT565';

  <<__LateInit>>
  private AliasCT565 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT565 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT565 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT565>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT565>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT565 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT565> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT566 = dict<arraykey, mixed>|arraykey;
              type AliasCT566 = CT566;

  
class CheckAliasCT566<T as AliasCT566> extends BaseCheck {
  const type T = AliasCT566;
  const string NAME = 'AliasCT566';

  <<__LateInit>>
  private AliasCT566 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT566 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT566 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT566>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT566>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT566 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT566> {
    return vec['','hello world',0,1,dict[]];
  }
}
case type CT567 = dynamic|arraykey;
              type AliasCT567 = CT567;

  
class CheckAliasCT567<T as AliasCT567> extends BaseCheck {
  const type T = AliasCT567;
  const string NAME = 'AliasCT567';

  <<__LateInit>>
  private AliasCT567 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT567 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT567 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT567>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT567>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT567 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT567> {
    return vec['','hello world',0,1,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT568 = float|arraykey;
              type AliasCT568 = CT568;

  
class CheckAliasCT568<T as AliasCT568> extends BaseCheck {
  const type T = AliasCT568;
  const string NAME = 'AliasCT568';

  <<__LateInit>>
  private AliasCT568 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT568 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT568 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT568>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT568>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT568 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT568> {
    return vec['','hello world',0,0.0,1,3.14];
  }
}
case type CT569 = int|arraykey;
              type AliasCT569 = CT569;

  
class CheckAliasCT569<T as AliasCT569> extends BaseCheck {
  const type T = AliasCT569;
  const string NAME = 'AliasCT569';

  <<__LateInit>>
  private AliasCT569 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT569 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT569 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT569>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT569>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT569 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT569> {
    return vec['','hello world',0,1];
  }
}
case type CT570 = keyset<arraykey>|arraykey;
              type AliasCT570 = CT570;

  
class CheckAliasCT570<T as AliasCT570> extends BaseCheck {
  const type T = AliasCT570;
  const string NAME = 'AliasCT570';

  <<__LateInit>>
  private AliasCT570 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT570 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT570 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT570>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT570>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT570 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT570> {
    return vec['','hello world',0,1,keyset[]];
  }
}
case type CT571 = mixed|arraykey;
              type AliasCT571 = CT571;

  
class CheckAliasCT571<T as AliasCT571> extends BaseCheck {
  const type T = AliasCT571;
  const string NAME = 'AliasCT571';

  <<__LateInit>>
  private AliasCT571 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT571 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT571 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT571>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT571>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT571 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT571> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT572 = nonnull|arraykey;
              type AliasCT572 = CT572;

  
class CheckAliasCT572<T as AliasCT572> extends BaseCheck {
  const type T = AliasCT572;
  const string NAME = 'AliasCT572';

  <<__LateInit>>
  private AliasCT572 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT572 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT572 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT572>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT572>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT572 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT572> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT573 = noreturn|arraykey;
              type AliasCT573 = CT573;

  
class CheckAliasCT573<T as AliasCT573> extends BaseCheck {
  const type T = AliasCT573;
  const string NAME = 'AliasCT573';

  <<__LateInit>>
  private AliasCT573 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT573 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT573 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT573>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT573>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT573 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT573> {
    return vec['','hello world',0,1];
  }
}
case type CT574 = nothing|arraykey;
              type AliasCT574 = CT574;

  
class CheckAliasCT574<T as AliasCT574> extends BaseCheck {
  const type T = AliasCT574;
  const string NAME = 'AliasCT574';

  <<__LateInit>>
  private AliasCT574 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT574 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT574 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT574>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT574>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT574 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT574> {
    return vec['','hello world',0,1];
  }
}
case type CT575 = null|arraykey;
              type AliasCT575 = CT575;

  
class CheckAliasCT575<T as AliasCT575> extends BaseCheck {
  const type T = AliasCT575;
  const string NAME = 'AliasCT575';

  <<__LateInit>>
  private AliasCT575 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT575 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT575 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT575>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT575>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT575 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT575> {
    return vec['','hello world',0,1,null];
  }
}
case type CT576 = num|arraykey;
              type AliasCT576 = CT576;

  
class CheckAliasCT576<T as AliasCT576> extends BaseCheck {
  const type T = AliasCT576;
  const string NAME = 'AliasCT576';

  <<__LateInit>>
  private AliasCT576 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT576 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT576 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT576>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT576>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT576 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT576> {
    return vec['','hello world',0,0.0,1,3.14];
  }
}
case type CT577 = resource|arraykey;
              type AliasCT577 = CT577;

  
class CheckAliasCT577<T as AliasCT577> extends BaseCheck {
  const type T = AliasCT577;
  const string NAME = 'AliasCT577';

  <<__LateInit>>
  private AliasCT577 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT577 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT577 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT577>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT577>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT577 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT577> {
    return vec['','hello world',0,1,imagecreate(10, 10)];
  }
}
case type CT578 = shape(...)|arraykey;
              type AliasCT578 = CT578;

  
class CheckAliasCT578<T as AliasCT578> extends BaseCheck {
  const type T = AliasCT578;
  const string NAME = 'AliasCT578';

  <<__LateInit>>
  private AliasCT578 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT578 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT578 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT578>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT578>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT578 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT578> {
    return vec['','hello world',0,1,shape('x' => 10),shape()];
  }
}
case type CT579 = string|arraykey;
              type AliasCT579 = CT579;

  
class CheckAliasCT579<T as AliasCT579> extends BaseCheck {
  const type T = AliasCT579;
  const string NAME = 'AliasCT579';

  <<__LateInit>>
  private AliasCT579 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT579 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT579 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT579>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT579>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT579 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT579> {
    return vec['','hello world',0,1];
  }
}
case type CT580 = vec<mixed>|arraykey;
              type AliasCT580 = CT580;

  
class CheckAliasCT580<T as AliasCT580> extends BaseCheck {
  const type T = AliasCT580;
  const string NAME = 'AliasCT580';

  <<__LateInit>>
  private AliasCT580 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT580 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT580 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT580>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT580>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT580 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT580> {
    return vec['','hello world',0,1,vec[]];
  }
}
case type CT581 = vec_or_dict<string>|arraykey;
              type AliasCT581 = CT581;

  
class CheckAliasCT581<T as AliasCT581> extends BaseCheck {
  const type T = AliasCT581;
  const string NAME = 'AliasCT581';

  <<__LateInit>>
  private AliasCT581 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT581 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT581 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT581>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT581>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT581 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT581> {
    return vec['','hello world',0,1,dict[],vec[]];
  }
}
case type CT582 = void|arraykey;
              type AliasCT582 = CT582;

  
class CheckAliasCT582<T as AliasCT582> extends BaseCheck {
  const type T = AliasCT582;
  const string NAME = 'AliasCT582';

  <<__LateInit>>
  private AliasCT582 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT582 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT582 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT582>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT582>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT582 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT582> {
    return vec['','hello world',0,1,null];
  }
}
case type CT583 = bool|bool;
              type AliasCT583 = CT583;

  
class CheckAliasCT583<T as AliasCT583> extends BaseCheck {
  const type T = AliasCT583;
  const string NAME = 'AliasCT583';

  <<__LateInit>>
  private AliasCT583 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT583 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT583 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT583>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT583>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT583 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT583> {
    return vec[false,true];
  }
}
case type CT584 = dict<arraykey, mixed>|bool;
              type AliasCT584 = CT584;

  
class CheckAliasCT584<T as AliasCT584> extends BaseCheck {
  const type T = AliasCT584;
  const string NAME = 'AliasCT584';

  <<__LateInit>>
  private AliasCT584 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT584 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT584 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT584>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT584>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT584 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT584> {
    return vec[dict[],false,true];
  }
}
case type CT585 = dynamic|bool;
              type AliasCT585 = CT585;

  
class CheckAliasCT585<T as AliasCT585> extends BaseCheck {
  const type T = AliasCT585;
  const string NAME = 'AliasCT585';

  <<__LateInit>>
  private AliasCT585 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT585 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT585 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT585>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT585>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT585 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT585> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT586 = float|bool;
              type AliasCT586 = CT586;

  
class CheckAliasCT586<T as AliasCT586> extends BaseCheck {
  const type T = AliasCT586;
  const string NAME = 'AliasCT586';

  <<__LateInit>>
  private AliasCT586 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT586 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT586 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT586>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT586>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT586 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT586> {
    return vec[0.0,3.14,false,true];
  }
}
case type CT587 = int|bool;
              type AliasCT587 = CT587;

  
class CheckAliasCT587<T as AliasCT587> extends BaseCheck {
  const type T = AliasCT587;
  const string NAME = 'AliasCT587';

  <<__LateInit>>
  private AliasCT587 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT587 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT587 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT587>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT587>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT587 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT587> {
    return vec[0,1,false,true];
  }
}
case type CT588 = keyset<arraykey>|bool;
              type AliasCT588 = CT588;

  
class CheckAliasCT588<T as AliasCT588> extends BaseCheck {
  const type T = AliasCT588;
  const string NAME = 'AliasCT588';

  <<__LateInit>>
  private AliasCT588 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT588 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT588 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT588>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT588>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT588 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT588> {
    return vec[false,keyset[],true];
  }
}
case type CT589 = mixed|bool;
              type AliasCT589 = CT589;

  
class CheckAliasCT589<T as AliasCT589> extends BaseCheck {
  const type T = AliasCT589;
  const string NAME = 'AliasCT589';

  <<__LateInit>>
  private AliasCT589 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT589 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT589 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT589>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT589>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT589 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT589> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT590 = nonnull|bool;
              type AliasCT590 = CT590;

  
class CheckAliasCT590<T as AliasCT590> extends BaseCheck {
  const type T = AliasCT590;
  const string NAME = 'AliasCT590';

  <<__LateInit>>
  private AliasCT590 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT590 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT590 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT590>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT590>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT590 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT590> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT591 = noreturn|bool;
              type AliasCT591 = CT591;

  
class CheckAliasCT591<T as AliasCT591> extends BaseCheck {
  const type T = AliasCT591;
  const string NAME = 'AliasCT591';

  <<__LateInit>>
  private AliasCT591 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT591 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT591 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT591>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT591>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT591 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT591> {
    return vec[false,true];
  }
}
case type CT592 = nothing|bool;
              type AliasCT592 = CT592;

  
class CheckAliasCT592<T as AliasCT592> extends BaseCheck {
  const type T = AliasCT592;
  const string NAME = 'AliasCT592';

  <<__LateInit>>
  private AliasCT592 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT592 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT592 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT592>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT592>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT592 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT592> {
    return vec[false,true];
  }
}
case type CT593 = null|bool;
              type AliasCT593 = CT593;

  
class CheckAliasCT593<T as AliasCT593> extends BaseCheck {
  const type T = AliasCT593;
  const string NAME = 'AliasCT593';

  <<__LateInit>>
  private AliasCT593 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT593 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT593 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT593>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT593>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT593 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT593> {
    return vec[false,null,true];
  }
}
case type CT594 = num|bool;
              type AliasCT594 = CT594;

  
class CheckAliasCT594<T as AliasCT594> extends BaseCheck {
  const type T = AliasCT594;
  const string NAME = 'AliasCT594';

  <<__LateInit>>
  private AliasCT594 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT594 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT594 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT594>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT594>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT594 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT594> {
    return vec[0,0.0,1,3.14,false,true];
  }
}
case type CT595 = resource|bool;
              type AliasCT595 = CT595;

  
class CheckAliasCT595<T as AliasCT595> extends BaseCheck {
  const type T = AliasCT595;
  const string NAME = 'AliasCT595';

  <<__LateInit>>
  private AliasCT595 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT595 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT595 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT595>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT595>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT595 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT595> {
    return vec[false,imagecreate(10, 10),true];
  }
}
case type CT596 = shape(...)|bool;
              type AliasCT596 = CT596;

  
class CheckAliasCT596<T as AliasCT596> extends BaseCheck {
  const type T = AliasCT596;
  const string NAME = 'AliasCT596';

  <<__LateInit>>
  private AliasCT596 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT596 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT596 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT596>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT596>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT596 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT596> {
    return vec[false,shape('x' => 10),shape(),true];
  }
}
case type CT597 = string|bool;
              type AliasCT597 = CT597;

  
class CheckAliasCT597<T as AliasCT597> extends BaseCheck {
  const type T = AliasCT597;
  const string NAME = 'AliasCT597';

  <<__LateInit>>
  private AliasCT597 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT597 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT597 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT597>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT597>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT597 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT597> {
    return vec['','hello world',false,true];
  }
}
case type CT598 = vec<mixed>|bool;
              type AliasCT598 = CT598;

  
class CheckAliasCT598<T as AliasCT598> extends BaseCheck {
  const type T = AliasCT598;
  const string NAME = 'AliasCT598';

  <<__LateInit>>
  private AliasCT598 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT598 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT598 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT598>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT598>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT598 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT598> {
    return vec[false,true,vec[]];
  }
}
case type CT599 = vec_or_dict<string>|bool;
              type AliasCT599 = CT599;

  
class CheckAliasCT599<T as AliasCT599> extends BaseCheck {
  const type T = AliasCT599;
  const string NAME = 'AliasCT599';

  <<__LateInit>>
  private AliasCT599 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT599 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT599 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT599>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT599>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT599 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT599> {
    return vec[dict[],false,true,vec[]];
  }
}
case type CT600 = void|bool;
              type AliasCT600 = CT600;

  
class CheckAliasCT600<T as AliasCT600> extends BaseCheck {
  const type T = AliasCT600;
  const string NAME = 'AliasCT600';

  <<__LateInit>>
  private AliasCT600 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT600 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT600 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT600>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT600>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT600 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT600> {
    return vec[false,null,true];
  }
}
case type CT601 = dict<arraykey, mixed>|dict<arraykey, mixed>;
              type AliasCT601 = CT601;

  
class CheckAliasCT601<T as AliasCT601> extends BaseCheck {
  const type T = AliasCT601;
  const string NAME = 'AliasCT601';

  <<__LateInit>>
  private AliasCT601 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT601 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT601 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT601>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT601>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT601 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT601> {
    return vec[dict[]];
  }
}
case type CT602 = dynamic|dict<arraykey, mixed>;
              type AliasCT602 = CT602;

  
class CheckAliasCT602<T as AliasCT602> extends BaseCheck {
  const type T = AliasCT602;
  const string NAME = 'AliasCT602';

  <<__LateInit>>
  private AliasCT602 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT602 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT602 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT602>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT602>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT602 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT602> {
    return vec[dict[],false,null,shape('x' => 10),shape(),true];
  }
}
case type CT603 = float|dict<arraykey, mixed>;
              type AliasCT603 = CT603;

  
class CheckAliasCT603<T as AliasCT603> extends BaseCheck {
  const type T = AliasCT603;
  const string NAME = 'AliasCT603';

  <<__LateInit>>
  private AliasCT603 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT603 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT603 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT603>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT603>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT603 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT603> {
    return vec[0.0,3.14,dict[]];
  }
}
case type CT604 = int|dict<arraykey, mixed>;
              type AliasCT604 = CT604;

  
class CheckAliasCT604<T as AliasCT604> extends BaseCheck {
  const type T = AliasCT604;
  const string NAME = 'AliasCT604';

  <<__LateInit>>
  private AliasCT604 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT604 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT604 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT604>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT604>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT604 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT604> {
    return vec[0,1,dict[]];
  }
}
case type CT605 = keyset<arraykey>|dict<arraykey, mixed>;
              type AliasCT605 = CT605;

  
class CheckAliasCT605<T as AliasCT605> extends BaseCheck {
  const type T = AliasCT605;
  const string NAME = 'AliasCT605';

  <<__LateInit>>
  private AliasCT605 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT605 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT605 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT605>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT605>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT605 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT605> {
    return vec[dict[],keyset[]];
  }
}
case type CT606 = mixed|dict<arraykey, mixed>;
              type AliasCT606 = CT606;

  
class CheckAliasCT606<T as AliasCT606> extends BaseCheck {
  const type T = AliasCT606;
  const string NAME = 'AliasCT606';

  <<__LateInit>>
  private AliasCT606 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT606 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT606 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT606>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT606>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT606 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT606> {
    return vec['','hello world',0,1,dict[],false,null,true];
  }
}
case type CT607 = nonnull|dict<arraykey, mixed>;
              type AliasCT607 = CT607;

  
class CheckAliasCT607<T as AliasCT607> extends BaseCheck {
  const type T = AliasCT607;
  const string NAME = 'AliasCT607';

  <<__LateInit>>
  private AliasCT607 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT607 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT607 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT607>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT607>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT607 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT607> {
    return vec['','hello world',0,1,dict[],false,true];
  }
}
case type CT608 = noreturn|dict<arraykey, mixed>;
              type AliasCT608 = CT608;

  
class CheckAliasCT608<T as AliasCT608> extends BaseCheck {
  const type T = AliasCT608;
  const string NAME = 'AliasCT608';

  <<__LateInit>>
  private AliasCT608 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT608 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT608 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT608>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT608>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT608 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT608> {
    return vec[dict[]];
  }
}
case type CT609 = nothing|dict<arraykey, mixed>;
              type AliasCT609 = CT609;

  
class CheckAliasCT609<T as AliasCT609> extends BaseCheck {
  const type T = AliasCT609;
  const string NAME = 'AliasCT609';

  <<__LateInit>>
  private AliasCT609 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT609 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT609 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT609>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT609>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT609 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT609> {
    return vec[dict[]];
  }
}
case type CT610 = null|dict<arraykey, mixed>;
              type AliasCT610 = CT610;

  
class CheckAliasCT610<T as AliasCT610> extends BaseCheck {
  const type T = AliasCT610;
  const string NAME = 'AliasCT610';

  <<__LateInit>>
  private AliasCT610 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT610 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT610 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT610>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT610>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT610 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT610> {
    return vec[dict[],null];
  }
}
case type CT611 = num|dict<arraykey, mixed>;
              type AliasCT611 = CT611;

  
class CheckAliasCT611<T as AliasCT611> extends BaseCheck {
  const type T = AliasCT611;
  const string NAME = 'AliasCT611';

  <<__LateInit>>
  private AliasCT611 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT611 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT611 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT611>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT611>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT611 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT611> {
    return vec[0,0.0,1,3.14,dict[]];
  }
}
case type CT612 = resource|dict<arraykey, mixed>;
              type AliasCT612 = CT612;

  
class CheckAliasCT612<T as AliasCT612> extends BaseCheck {
  const type T = AliasCT612;
  const string NAME = 'AliasCT612';

  <<__LateInit>>
  private AliasCT612 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT612 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT612 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT612>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT612>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT612 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT612> {
    return vec[dict[],imagecreate(10, 10)];
  }
}
case type CT613 = shape(...)|dict<arraykey, mixed>;
              type AliasCT613 = CT613;

  
class CheckAliasCT613<T as AliasCT613> extends BaseCheck {
  const type T = AliasCT613;
  const string NAME = 'AliasCT613';

  <<__LateInit>>
  private AliasCT613 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT613 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT613 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT613>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT613>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT613 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT613> {
    return vec[dict[],shape('x' => 10),shape()];
  }
}
case type CT614 = string|dict<arraykey, mixed>;
              type AliasCT614 = CT614;

  
class CheckAliasCT614<T as AliasCT614> extends BaseCheck {
  const type T = AliasCT614;
  const string NAME = 'AliasCT614';

  <<__LateInit>>
  private AliasCT614 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT614 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT614 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT614>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT614>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT614 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT614> {
    return vec['','hello world',dict[]];
  }
}
case type CT615 = vec<mixed>|dict<arraykey, mixed>;
              type AliasCT615 = CT615;

  
class CheckAliasCT615<T as AliasCT615> extends BaseCheck {
  const type T = AliasCT615;
  const string NAME = 'AliasCT615';

  <<__LateInit>>
  private AliasCT615 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT615 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT615 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT615>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT615>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT615 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT615> {
    return vec[dict[],vec[]];
  }
}
case type CT616 = vec_or_dict<string>|dict<arraykey, mixed>;
              type AliasCT616 = CT616;

  
class CheckAliasCT616<T as AliasCT616> extends BaseCheck {
  const type T = AliasCT616;
  const string NAME = 'AliasCT616';

  <<__LateInit>>
  private AliasCT616 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT616 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT616 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT616>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT616>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT616 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT616> {
    return vec[dict[],vec[]];
  }
}
case type CT617 = void|dict<arraykey, mixed>;
              type AliasCT617 = CT617;

  
class CheckAliasCT617<T as AliasCT617> extends BaseCheck {
  const type T = AliasCT617;
  const string NAME = 'AliasCT617';

  <<__LateInit>>
  private AliasCT617 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT617 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT617 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT617>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT617>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT617 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT617> {
    return vec[dict[],null];
  }
}
case type CT618 = dynamic|dynamic;
              type AliasCT618 = CT618;

  
class CheckAliasCT618<T as AliasCT618> extends BaseCheck {
  const type T = AliasCT618;
  const string NAME = 'AliasCT618';

  <<__LateInit>>
  private AliasCT618 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT618 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT618 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT618>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT618>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT618 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT618> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT619 = float|dynamic;
              type AliasCT619 = CT619;

  
class CheckAliasCT619<T as AliasCT619> extends BaseCheck {
  const type T = AliasCT619;
  const string NAME = 'AliasCT619';

  <<__LateInit>>
  private AliasCT619 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT619 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT619 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT619>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT619>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT619 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT619> {
    return vec[0.0,3.14,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT620 = int|dynamic;
              type AliasCT620 = CT620;

  
class CheckAliasCT620<T as AliasCT620> extends BaseCheck {
  const type T = AliasCT620;
  const string NAME = 'AliasCT620';

  <<__LateInit>>
  private AliasCT620 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT620 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT620 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT620>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT620>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT620 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT620> {
    return vec[0,1,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT621 = keyset<arraykey>|dynamic;
              type AliasCT621 = CT621;

  
class CheckAliasCT621<T as AliasCT621> extends BaseCheck {
  const type T = AliasCT621;
  const string NAME = 'AliasCT621';

  <<__LateInit>>
  private AliasCT621 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT621 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT621 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT621>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT621>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT621 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT621> {
    return vec[false,keyset[],null,shape('x' => 10),shape(),true];
  }
}
case type CT622 = mixed|dynamic;
              type AliasCT622 = CT622;

  
class CheckAliasCT622<T as AliasCT622> extends BaseCheck {
  const type T = AliasCT622;
  const string NAME = 'AliasCT622';

  <<__LateInit>>
  private AliasCT622 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT622 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT622 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT622>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT622>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT622 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT622> {
    return vec['','hello world',0,1,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT623 = nonnull|dynamic;
              type AliasCT623 = CT623;

  
class CheckAliasCT623<T as AliasCT623> extends BaseCheck {
  const type T = AliasCT623;
  const string NAME = 'AliasCT623';

  <<__LateInit>>
  private AliasCT623 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT623 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT623 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT623>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT623>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT623 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT623> {
    return vec['','hello world',0,1,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT624 = noreturn|dynamic;
              type AliasCT624 = CT624;

  
class CheckAliasCT624<T as AliasCT624> extends BaseCheck {
  const type T = AliasCT624;
  const string NAME = 'AliasCT624';

  <<__LateInit>>
  private AliasCT624 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT624 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT624 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT624>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT624>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT624 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT624> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT625 = nothing|dynamic;
              type AliasCT625 = CT625;

  
class CheckAliasCT625<T as AliasCT625> extends BaseCheck {
  const type T = AliasCT625;
  const string NAME = 'AliasCT625';

  <<__LateInit>>
  private AliasCT625 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT625 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT625 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT625>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT625>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT625 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT625> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT626 = null|dynamic;
              type AliasCT626 = CT626;

  
class CheckAliasCT626<T as AliasCT626> extends BaseCheck {
  const type T = AliasCT626;
  const string NAME = 'AliasCT626';

  <<__LateInit>>
  private AliasCT626 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT626 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT626 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT626>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT626>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT626 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT626> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT627 = num|dynamic;
              type AliasCT627 = CT627;

  
class CheckAliasCT627<T as AliasCT627> extends BaseCheck {
  const type T = AliasCT627;
  const string NAME = 'AliasCT627';

  <<__LateInit>>
  private AliasCT627 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT627 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT627 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT627>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT627>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT627 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT627> {
    return vec[0,0.0,1,3.14,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT628 = resource|dynamic;
              type AliasCT628 = CT628;

  
class CheckAliasCT628<T as AliasCT628> extends BaseCheck {
  const type T = AliasCT628;
  const string NAME = 'AliasCT628';

  <<__LateInit>>
  private AliasCT628 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT628 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT628 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT628>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT628>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT628 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT628> {
    return vec[false,imagecreate(10, 10),null,shape('x' => 10),shape(),true];
  }
}
case type CT629 = shape(...)|dynamic;
              type AliasCT629 = CT629;

  
class CheckAliasCT629<T as AliasCT629> extends BaseCheck {
  const type T = AliasCT629;
  const string NAME = 'AliasCT629';

  <<__LateInit>>
  private AliasCT629 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT629 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT629 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT629>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT629>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT629 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT629> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT630 = string|dynamic;
              type AliasCT630 = CT630;

  
class CheckAliasCT630<T as AliasCT630> extends BaseCheck {
  const type T = AliasCT630;
  const string NAME = 'AliasCT630';

  <<__LateInit>>
  private AliasCT630 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT630 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT630 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT630>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT630>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT630 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT630> {
    return vec['','hello world',false,null,shape('x' => 10),shape(),true];
  }
}
case type CT631 = vec<mixed>|dynamic;
              type AliasCT631 = CT631;

  
class CheckAliasCT631<T as AliasCT631> extends BaseCheck {
  const type T = AliasCT631;
  const string NAME = 'AliasCT631';

  <<__LateInit>>
  private AliasCT631 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT631 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT631 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT631>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT631>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT631 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT631> {
    return vec[false,null,shape('x' => 10),shape(),true,vec[]];
  }
}
case type CT632 = vec_or_dict<string>|dynamic;
              type AliasCT632 = CT632;

  
class CheckAliasCT632<T as AliasCT632> extends BaseCheck {
  const type T = AliasCT632;
  const string NAME = 'AliasCT632';

  <<__LateInit>>
  private AliasCT632 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT632 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT632 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT632>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT632>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT632 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT632> {
    return vec[dict[],false,null,shape('x' => 10),shape(),true,vec[]];
  }
}
case type CT633 = void|dynamic;
              type AliasCT633 = CT633;

  
class CheckAliasCT633<T as AliasCT633> extends BaseCheck {
  const type T = AliasCT633;
  const string NAME = 'AliasCT633';

  <<__LateInit>>
  private AliasCT633 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT633 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT633 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT633>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT633>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT633 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT633> {
    return vec[false,null,shape('x' => 10),shape(),true];
  }
}
case type CT634 = float|float;
              type AliasCT634 = CT634;

  
class CheckAliasCT634<T as AliasCT634> extends BaseCheck {
  const type T = AliasCT634;
  const string NAME = 'AliasCT634';

  <<__LateInit>>
  private AliasCT634 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT634 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT634 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT634>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT634>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT634 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT634> {
    return vec[0.0,3.14];
  }
}
case type CT635 = int|float;
              type AliasCT635 = CT635;

  
class CheckAliasCT635<T as AliasCT635> extends BaseCheck {
  const type T = AliasCT635;
  const string NAME = 'AliasCT635';

  <<__LateInit>>
  private AliasCT635 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT635 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT635 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT635>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT635>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT635 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT635> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT636 = keyset<arraykey>|float;
              type AliasCT636 = CT636;

  
class CheckAliasCT636<T as AliasCT636> extends BaseCheck {
  const type T = AliasCT636;
  const string NAME = 'AliasCT636';

  <<__LateInit>>
  private AliasCT636 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT636 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT636 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT636>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT636>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT636 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT636> {
    return vec[0.0,3.14,keyset[]];
  }
}
case type CT637 = mixed|float;
              type AliasCT637 = CT637;

  
class CheckAliasCT637<T as AliasCT637> extends BaseCheck {
  const type T = AliasCT637;
  const string NAME = 'AliasCT637';

  <<__LateInit>>
  private AliasCT637 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT637 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT637 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT637>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT637>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT637 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT637> {
    return vec['','hello world',0,0.0,1,3.14,false,null,true];
  }
}
case type CT638 = nonnull|float;
              type AliasCT638 = CT638;

  
class CheckAliasCT638<T as AliasCT638> extends BaseCheck {
  const type T = AliasCT638;
  const string NAME = 'AliasCT638';

  <<__LateInit>>
  private AliasCT638 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT638 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT638 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT638>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT638>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT638 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT638> {
    return vec['','hello world',0,0.0,1,3.14,false,true];
  }
}
case type CT639 = noreturn|float;
              type AliasCT639 = CT639;

  
class CheckAliasCT639<T as AliasCT639> extends BaseCheck {
  const type T = AliasCT639;
  const string NAME = 'AliasCT639';

  <<__LateInit>>
  private AliasCT639 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT639 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT639 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT639>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT639>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT639 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT639> {
    return vec[0.0,3.14];
  }
}
case type CT640 = nothing|float;
              type AliasCT640 = CT640;

  
class CheckAliasCT640<T as AliasCT640> extends BaseCheck {
  const type T = AliasCT640;
  const string NAME = 'AliasCT640';

  <<__LateInit>>
  private AliasCT640 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT640 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT640 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT640>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT640>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT640 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT640> {
    return vec[0.0,3.14];
  }
}
case type CT641 = null|float;
              type AliasCT641 = CT641;

  
class CheckAliasCT641<T as AliasCT641> extends BaseCheck {
  const type T = AliasCT641;
  const string NAME = 'AliasCT641';

  <<__LateInit>>
  private AliasCT641 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT641 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT641 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT641>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT641>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT641 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT641> {
    return vec[0.0,3.14,null];
  }
}
case type CT642 = num|float;
              type AliasCT642 = CT642;

  
class CheckAliasCT642<T as AliasCT642> extends BaseCheck {
  const type T = AliasCT642;
  const string NAME = 'AliasCT642';

  <<__LateInit>>
  private AliasCT642 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT642 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT642 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT642>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT642>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT642 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT642> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT643 = resource|float;
              type AliasCT643 = CT643;

  
class CheckAliasCT643<T as AliasCT643> extends BaseCheck {
  const type T = AliasCT643;
  const string NAME = 'AliasCT643';

  <<__LateInit>>
  private AliasCT643 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT643 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT643 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT643>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT643>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT643 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT643> {
    return vec[0.0,3.14,imagecreate(10, 10)];
  }
}
case type CT644 = shape(...)|float;
              type AliasCT644 = CT644;

  
class CheckAliasCT644<T as AliasCT644> extends BaseCheck {
  const type T = AliasCT644;
  const string NAME = 'AliasCT644';

  <<__LateInit>>
  private AliasCT644 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT644 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT644 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT644>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT644>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT644 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT644> {
    return vec[0.0,3.14,shape('x' => 10),shape()];
  }
}
case type CT645 = string|float;
              type AliasCT645 = CT645;

  
class CheckAliasCT645<T as AliasCT645> extends BaseCheck {
  const type T = AliasCT645;
  const string NAME = 'AliasCT645';

  <<__LateInit>>
  private AliasCT645 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT645 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT645 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT645>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT645>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT645 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT645> {
    return vec['','hello world',0.0,3.14];
  }
}
case type CT646 = vec<mixed>|float;
              type AliasCT646 = CT646;

  
class CheckAliasCT646<T as AliasCT646> extends BaseCheck {
  const type T = AliasCT646;
  const string NAME = 'AliasCT646';

  <<__LateInit>>
  private AliasCT646 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT646 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT646 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT646>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT646>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT646 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT646> {
    return vec[0.0,3.14,vec[]];
  }
}
case type CT647 = vec_or_dict<string>|float;
              type AliasCT647 = CT647;

  
class CheckAliasCT647<T as AliasCT647> extends BaseCheck {
  const type T = AliasCT647;
  const string NAME = 'AliasCT647';

  <<__LateInit>>
  private AliasCT647 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT647 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT647 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT647>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT647>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT647 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT647> {
    return vec[0.0,3.14,dict[],vec[]];
  }
}
case type CT648 = void|float;
              type AliasCT648 = CT648;

  
class CheckAliasCT648<T as AliasCT648> extends BaseCheck {
  const type T = AliasCT648;
  const string NAME = 'AliasCT648';

  <<__LateInit>>
  private AliasCT648 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT648 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT648 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT648>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT648>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT648 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT648> {
    return vec[0.0,3.14,null];
  }
}
case type CT649 = int|int;
              type AliasCT649 = CT649;

  
class CheckAliasCT649<T as AliasCT649> extends BaseCheck {
  const type T = AliasCT649;
  const string NAME = 'AliasCT649';

  <<__LateInit>>
  private AliasCT649 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT649 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT649 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT649>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT649>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT649 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT649> {
    return vec[0,1];
  }
}
case type CT650 = keyset<arraykey>|int;
              type AliasCT650 = CT650;

  
class CheckAliasCT650<T as AliasCT650> extends BaseCheck {
  const type T = AliasCT650;
  const string NAME = 'AliasCT650';

  <<__LateInit>>
  private AliasCT650 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT650 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT650 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT650>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT650>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT650 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT650> {
    return vec[0,1,keyset[]];
  }
}
case type CT651 = mixed|int;
              type AliasCT651 = CT651;

  
class CheckAliasCT651<T as AliasCT651> extends BaseCheck {
  const type T = AliasCT651;
  const string NAME = 'AliasCT651';

  <<__LateInit>>
  private AliasCT651 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT651 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT651 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT651>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT651>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT651 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT651> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT652 = nonnull|int;
              type AliasCT652 = CT652;

  
class CheckAliasCT652<T as AliasCT652> extends BaseCheck {
  const type T = AliasCT652;
  const string NAME = 'AliasCT652';

  <<__LateInit>>
  private AliasCT652 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT652 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT652 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT652>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT652>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT652 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT652> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT653 = noreturn|int;
              type AliasCT653 = CT653;

  
class CheckAliasCT653<T as AliasCT653> extends BaseCheck {
  const type T = AliasCT653;
  const string NAME = 'AliasCT653';

  <<__LateInit>>
  private AliasCT653 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT653 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT653 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT653>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT653>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT653 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT653> {
    return vec[0,1];
  }
}
case type CT654 = nothing|int;
              type AliasCT654 = CT654;

  
class CheckAliasCT654<T as AliasCT654> extends BaseCheck {
  const type T = AliasCT654;
  const string NAME = 'AliasCT654';

  <<__LateInit>>
  private AliasCT654 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT654 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT654 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT654>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT654>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT654 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT654> {
    return vec[0,1];
  }
}
case type CT655 = null|int;
              type AliasCT655 = CT655;

  
class CheckAliasCT655<T as AliasCT655> extends BaseCheck {
  const type T = AliasCT655;
  const string NAME = 'AliasCT655';

  <<__LateInit>>
  private AliasCT655 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT655 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT655 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT655>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT655>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT655 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT655> {
    return vec[0,1,null];
  }
}
case type CT656 = num|int;
              type AliasCT656 = CT656;

  
class CheckAliasCT656<T as AliasCT656> extends BaseCheck {
  const type T = AliasCT656;
  const string NAME = 'AliasCT656';

  <<__LateInit>>
  private AliasCT656 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT656 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT656 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT656>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT656>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT656 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT656> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT657 = resource|int;
              type AliasCT657 = CT657;

  
class CheckAliasCT657<T as AliasCT657> extends BaseCheck {
  const type T = AliasCT657;
  const string NAME = 'AliasCT657';

  <<__LateInit>>
  private AliasCT657 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT657 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT657 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT657>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT657>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT657 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT657> {
    return vec[0,1,imagecreate(10, 10)];
  }
}
case type CT658 = shape(...)|int;
              type AliasCT658 = CT658;

  
class CheckAliasCT658<T as AliasCT658> extends BaseCheck {
  const type T = AliasCT658;
  const string NAME = 'AliasCT658';

  <<__LateInit>>
  private AliasCT658 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT658 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT658 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT658>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT658>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT658 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT658> {
    return vec[0,1,shape('x' => 10),shape()];
  }
}
case type CT659 = string|int;
              type AliasCT659 = CT659;

  
class CheckAliasCT659<T as AliasCT659> extends BaseCheck {
  const type T = AliasCT659;
  const string NAME = 'AliasCT659';

  <<__LateInit>>
  private AliasCT659 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT659 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT659 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT659>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT659>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT659 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT659> {
    return vec['','hello world',0,1];
  }
}
case type CT660 = vec<mixed>|int;
              type AliasCT660 = CT660;

  
class CheckAliasCT660<T as AliasCT660> extends BaseCheck {
  const type T = AliasCT660;
  const string NAME = 'AliasCT660';

  <<__LateInit>>
  private AliasCT660 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT660 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT660 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT660>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT660>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT660 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT660> {
    return vec[0,1,vec[]];
  }
}
case type CT661 = vec_or_dict<string>|int;
              type AliasCT661 = CT661;

  
class CheckAliasCT661<T as AliasCT661> extends BaseCheck {
  const type T = AliasCT661;
  const string NAME = 'AliasCT661';

  <<__LateInit>>
  private AliasCT661 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT661 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT661 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT661>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT661>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT661 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT661> {
    return vec[0,1,dict[],vec[]];
  }
}
case type CT662 = void|int;
              type AliasCT662 = CT662;

  
class CheckAliasCT662<T as AliasCT662> extends BaseCheck {
  const type T = AliasCT662;
  const string NAME = 'AliasCT662';

  <<__LateInit>>
  private AliasCT662 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT662 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT662 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT662>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT662>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT662 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT662> {
    return vec[0,1,null];
  }
}
case type CT663 = keyset<arraykey>|keyset<arraykey>;
              type AliasCT663 = CT663;

  
class CheckAliasCT663<T as AliasCT663> extends BaseCheck {
  const type T = AliasCT663;
  const string NAME = 'AliasCT663';

  <<__LateInit>>
  private AliasCT663 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT663 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT663 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT663>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT663>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT663 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT663> {
    return vec[keyset[]];
  }
}
case type CT664 = mixed|keyset<arraykey>;
              type AliasCT664 = CT664;

  
class CheckAliasCT664<T as AliasCT664> extends BaseCheck {
  const type T = AliasCT664;
  const string NAME = 'AliasCT664';

  <<__LateInit>>
  private AliasCT664 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT664 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT664 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT664>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT664>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT664 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT664> {
    return vec['','hello world',0,1,false,keyset[],null,true];
  }
}
case type CT665 = nonnull|keyset<arraykey>;
              type AliasCT665 = CT665;

  
class CheckAliasCT665<T as AliasCT665> extends BaseCheck {
  const type T = AliasCT665;
  const string NAME = 'AliasCT665';

  <<__LateInit>>
  private AliasCT665 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT665 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT665 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT665>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT665>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT665 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT665> {
    return vec['','hello world',0,1,false,keyset[],true];
  }
}
case type CT666 = noreturn|keyset<arraykey>;
              type AliasCT666 = CT666;

  
class CheckAliasCT666<T as AliasCT666> extends BaseCheck {
  const type T = AliasCT666;
  const string NAME = 'AliasCT666';

  <<__LateInit>>
  private AliasCT666 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT666 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT666 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT666>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT666>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT666 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT666> {
    return vec[keyset[]];
  }
}
case type CT667 = nothing|keyset<arraykey>;
              type AliasCT667 = CT667;

  
class CheckAliasCT667<T as AliasCT667> extends BaseCheck {
  const type T = AliasCT667;
  const string NAME = 'AliasCT667';

  <<__LateInit>>
  private AliasCT667 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT667 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT667 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT667>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT667>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT667 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT667> {
    return vec[keyset[]];
  }
}
case type CT668 = null|keyset<arraykey>;
              type AliasCT668 = CT668;

  
class CheckAliasCT668<T as AliasCT668> extends BaseCheck {
  const type T = AliasCT668;
  const string NAME = 'AliasCT668';

  <<__LateInit>>
  private AliasCT668 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT668 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT668 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT668>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT668>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT668 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT668> {
    return vec[keyset[],null];
  }
}
case type CT669 = num|keyset<arraykey>;
              type AliasCT669 = CT669;

  
class CheckAliasCT669<T as AliasCT669> extends BaseCheck {
  const type T = AliasCT669;
  const string NAME = 'AliasCT669';

  <<__LateInit>>
  private AliasCT669 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT669 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT669 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT669>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT669>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT669 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT669> {
    return vec[0,0.0,1,3.14,keyset[]];
  }
}
case type CT670 = resource|keyset<arraykey>;
              type AliasCT670 = CT670;

  
class CheckAliasCT670<T as AliasCT670> extends BaseCheck {
  const type T = AliasCT670;
  const string NAME = 'AliasCT670';

  <<__LateInit>>
  private AliasCT670 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT670 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT670 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT670>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT670>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT670 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT670> {
    return vec[imagecreate(10, 10),keyset[]];
  }
}
case type CT671 = shape(...)|keyset<arraykey>;
              type AliasCT671 = CT671;

  
class CheckAliasCT671<T as AliasCT671> extends BaseCheck {
  const type T = AliasCT671;
  const string NAME = 'AliasCT671';

  <<__LateInit>>
  private AliasCT671 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT671 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT671 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT671>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT671>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT671 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT671> {
    return vec[keyset[],shape('x' => 10),shape()];
  }
}
case type CT672 = string|keyset<arraykey>;
              type AliasCT672 = CT672;

  
class CheckAliasCT672<T as AliasCT672> extends BaseCheck {
  const type T = AliasCT672;
  const string NAME = 'AliasCT672';

  <<__LateInit>>
  private AliasCT672 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT672 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT672 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT672>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT672>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT672 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT672> {
    return vec['','hello world',keyset[]];
  }
}
case type CT673 = vec<mixed>|keyset<arraykey>;
              type AliasCT673 = CT673;

  
class CheckAliasCT673<T as AliasCT673> extends BaseCheck {
  const type T = AliasCT673;
  const string NAME = 'AliasCT673';

  <<__LateInit>>
  private AliasCT673 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT673 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT673 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT673>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT673>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT673 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT673> {
    return vec[keyset[],vec[]];
  }
}
case type CT674 = vec_or_dict<string>|keyset<arraykey>;
              type AliasCT674 = CT674;

  
class CheckAliasCT674<T as AliasCT674> extends BaseCheck {
  const type T = AliasCT674;
  const string NAME = 'AliasCT674';

  <<__LateInit>>
  private AliasCT674 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT674 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT674 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT674>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT674>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT674 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT674> {
    return vec[dict[],keyset[],vec[]];
  }
}
case type CT675 = void|keyset<arraykey>;
              type AliasCT675 = CT675;

  
class CheckAliasCT675<T as AliasCT675> extends BaseCheck {
  const type T = AliasCT675;
  const string NAME = 'AliasCT675';

  <<__LateInit>>
  private AliasCT675 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT675 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT675 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT675>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT675>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT675 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT675> {
    return vec[keyset[],null];
  }
}
case type CT676 = mixed|mixed;
              type AliasCT676 = CT676;

  
class CheckAliasCT676<T as AliasCT676> extends BaseCheck {
  const type T = AliasCT676;
  const string NAME = 'AliasCT676';

  <<__LateInit>>
  private AliasCT676 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT676 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT676 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT676>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT676>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT676 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT676> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT677 = nonnull|mixed;
              type AliasCT677 = CT677;

  
class CheckAliasCT677<T as AliasCT677> extends BaseCheck {
  const type T = AliasCT677;
  const string NAME = 'AliasCT677';

  <<__LateInit>>
  private AliasCT677 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT677 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT677 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT677>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT677>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT677 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT677> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT678 = noreturn|mixed;
              type AliasCT678 = CT678;

  
class CheckAliasCT678<T as AliasCT678> extends BaseCheck {
  const type T = AliasCT678;
  const string NAME = 'AliasCT678';

  <<__LateInit>>
  private AliasCT678 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT678 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT678 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT678>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT678>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT678 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT678> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT679 = nothing|mixed;
              type AliasCT679 = CT679;

  
class CheckAliasCT679<T as AliasCT679> extends BaseCheck {
  const type T = AliasCT679;
  const string NAME = 'AliasCT679';

  <<__LateInit>>
  private AliasCT679 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT679 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT679 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT679>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT679>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT679 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT679> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT680 = null|mixed;
              type AliasCT680 = CT680;

  
class CheckAliasCT680<T as AliasCT680> extends BaseCheck {
  const type T = AliasCT680;
  const string NAME = 'AliasCT680';

  <<__LateInit>>
  private AliasCT680 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT680 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT680 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT680>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT680>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT680 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT680> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT681 = num|mixed;
              type AliasCT681 = CT681;

  
class CheckAliasCT681<T as AliasCT681> extends BaseCheck {
  const type T = AliasCT681;
  const string NAME = 'AliasCT681';

  <<__LateInit>>
  private AliasCT681 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT681 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT681 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT681>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT681>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT681 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT681> {
    return vec['','hello world',0,0.0,1,3.14,false,null,true];
  }
}
case type CT682 = resource|mixed;
              type AliasCT682 = CT682;

  
class CheckAliasCT682<T as AliasCT682> extends BaseCheck {
  const type T = AliasCT682;
  const string NAME = 'AliasCT682';

  <<__LateInit>>
  private AliasCT682 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT682 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT682 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT682>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT682>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT682 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT682> {
    return vec['','hello world',0,1,false,imagecreate(10, 10),null,true];
  }
}
case type CT683 = shape(...)|mixed;
              type AliasCT683 = CT683;

  
class CheckAliasCT683<T as AliasCT683> extends BaseCheck {
  const type T = AliasCT683;
  const string NAME = 'AliasCT683';

  <<__LateInit>>
  private AliasCT683 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT683 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT683 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT683>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT683>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT683 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT683> {
    return vec['','hello world',0,1,false,null,shape('x' => 10),shape(),true];
  }
}
case type CT684 = string|mixed;
              type AliasCT684 = CT684;

  
class CheckAliasCT684<T as AliasCT684> extends BaseCheck {
  const type T = AliasCT684;
  const string NAME = 'AliasCT684';

  <<__LateInit>>
  private AliasCT684 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT684 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT684 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT684>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT684>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT684 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT684> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT685 = vec<mixed>|mixed;
              type AliasCT685 = CT685;

  
class CheckAliasCT685<T as AliasCT685> extends BaseCheck {
  const type T = AliasCT685;
  const string NAME = 'AliasCT685';

  <<__LateInit>>
  private AliasCT685 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT685 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT685 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT685>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT685>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT685 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT685> {
    return vec['','hello world',0,1,false,null,true,vec[]];
  }
}
case type CT686 = vec_or_dict<string>|mixed;
              type AliasCT686 = CT686;

  
class CheckAliasCT686<T as AliasCT686> extends BaseCheck {
  const type T = AliasCT686;
  const string NAME = 'AliasCT686';

  <<__LateInit>>
  private AliasCT686 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT686 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT686 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT686>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT686>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT686 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT686> {
    return vec['','hello world',0,1,dict[],false,null,true,vec[]];
  }
}
case type CT687 = void|mixed;
              type AliasCT687 = CT687;

  
class CheckAliasCT687<T as AliasCT687> extends BaseCheck {
  const type T = AliasCT687;
  const string NAME = 'AliasCT687';

  <<__LateInit>>
  private AliasCT687 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT687 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT687 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT687>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT687>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT687 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT687> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT688 = nonnull|nonnull;
              type AliasCT688 = CT688;

  
class CheckAliasCT688<T as AliasCT688> extends BaseCheck {
  const type T = AliasCT688;
  const string NAME = 'AliasCT688';

  <<__LateInit>>
  private AliasCT688 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT688 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT688 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT688>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT688>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT688 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT688> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT689 = noreturn|nonnull;
              type AliasCT689 = CT689;

  
class CheckAliasCT689<T as AliasCT689> extends BaseCheck {
  const type T = AliasCT689;
  const string NAME = 'AliasCT689';

  <<__LateInit>>
  private AliasCT689 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT689 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT689 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT689>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT689>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT689 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT689> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT690 = nothing|nonnull;
              type AliasCT690 = CT690;

  
class CheckAliasCT690<T as AliasCT690> extends BaseCheck {
  const type T = AliasCT690;
  const string NAME = 'AliasCT690';

  <<__LateInit>>
  private AliasCT690 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT690 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT690 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT690>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT690>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT690 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT690> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT691 = null|nonnull;
              type AliasCT691 = CT691;

  
class CheckAliasCT691<T as AliasCT691> extends BaseCheck {
  const type T = AliasCT691;
  const string NAME = 'AliasCT691';

  <<__LateInit>>
  private AliasCT691 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT691 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT691 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT691>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT691>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT691 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT691> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT692 = num|nonnull;
              type AliasCT692 = CT692;

  
class CheckAliasCT692<T as AliasCT692> extends BaseCheck {
  const type T = AliasCT692;
  const string NAME = 'AliasCT692';

  <<__LateInit>>
  private AliasCT692 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT692 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT692 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT692>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT692>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT692 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT692> {
    return vec['','hello world',0,0.0,1,3.14,false,true];
  }
}
case type CT693 = resource|nonnull;
              type AliasCT693 = CT693;

  
class CheckAliasCT693<T as AliasCT693> extends BaseCheck {
  const type T = AliasCT693;
  const string NAME = 'AliasCT693';

  <<__LateInit>>
  private AliasCT693 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT693 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT693 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT693>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT693>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT693 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT693> {
    return vec['','hello world',0,1,false,imagecreate(10, 10),true];
  }
}
case type CT694 = shape(...)|nonnull;
              type AliasCT694 = CT694;

  
class CheckAliasCT694<T as AliasCT694> extends BaseCheck {
  const type T = AliasCT694;
  const string NAME = 'AliasCT694';

  <<__LateInit>>
  private AliasCT694 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT694 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT694 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT694>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT694>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT694 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT694> {
    return vec['','hello world',0,1,false,shape('x' => 10),shape(),true];
  }
}
case type CT695 = string|nonnull;
              type AliasCT695 = CT695;

  
class CheckAliasCT695<T as AliasCT695> extends BaseCheck {
  const type T = AliasCT695;
  const string NAME = 'AliasCT695';

  <<__LateInit>>
  private AliasCT695 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT695 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT695 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT695>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT695>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT695 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT695> {
    return vec['','hello world',0,1,false,true];
  }
}
case type CT696 = vec<mixed>|nonnull;
              type AliasCT696 = CT696;

  
class CheckAliasCT696<T as AliasCT696> extends BaseCheck {
  const type T = AliasCT696;
  const string NAME = 'AliasCT696';

  <<__LateInit>>
  private AliasCT696 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT696 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT696 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT696>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT696>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT696 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT696> {
    return vec['','hello world',0,1,false,true,vec[]];
  }
}
case type CT697 = vec_or_dict<string>|nonnull;
              type AliasCT697 = CT697;

  
class CheckAliasCT697<T as AliasCT697> extends BaseCheck {
  const type T = AliasCT697;
  const string NAME = 'AliasCT697';

  <<__LateInit>>
  private AliasCT697 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT697 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT697 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT697>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT697>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT697 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT697> {
    return vec['','hello world',0,1,dict[],false,true,vec[]];
  }
}
case type CT698 = void|nonnull;
              type AliasCT698 = CT698;

  
class CheckAliasCT698<T as AliasCT698> extends BaseCheck {
  const type T = AliasCT698;
  const string NAME = 'AliasCT698';

  <<__LateInit>>
  private AliasCT698 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT698 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT698 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT698>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT698>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT698 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT698> {
    return vec['','hello world',0,1,false,null,true];
  }
}
case type CT699 = noreturn|noreturn;
              type AliasCT699 = CT699;

  
class CheckAliasCT699<T as AliasCT699> extends BaseCheck {
  const type T = AliasCT699;
  const string NAME = 'AliasCT699';

  <<__LateInit>>
  private AliasCT699 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT699 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT699 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT699>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT699>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT699 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT699> {
    return vec[];
  }
}
case type CT700 = nothing|noreturn;
              type AliasCT700 = CT700;

  
class CheckAliasCT700<T as AliasCT700> extends BaseCheck {
  const type T = AliasCT700;
  const string NAME = 'AliasCT700';

  <<__LateInit>>
  private AliasCT700 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT700 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT700 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT700>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT700>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT700 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT700> {
    return vec[];
  }
}
case type CT701 = null|noreturn;
              type AliasCT701 = CT701;

  
class CheckAliasCT701<T as AliasCT701> extends BaseCheck {
  const type T = AliasCT701;
  const string NAME = 'AliasCT701';

  <<__LateInit>>
  private AliasCT701 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT701 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT701 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT701>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT701>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT701 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT701> {
    return vec[null];
  }
}
case type CT702 = num|noreturn;
              type AliasCT702 = CT702;

  
class CheckAliasCT702<T as AliasCT702> extends BaseCheck {
  const type T = AliasCT702;
  const string NAME = 'AliasCT702';

  <<__LateInit>>
  private AliasCT702 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT702 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT702 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT702>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT702>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT702 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT702> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT703 = resource|noreturn;
              type AliasCT703 = CT703;

  
class CheckAliasCT703<T as AliasCT703> extends BaseCheck {
  const type T = AliasCT703;
  const string NAME = 'AliasCT703';

  <<__LateInit>>
  private AliasCT703 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT703 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT703 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT703>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT703>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT703 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT703> {
    return vec[imagecreate(10, 10)];
  }
}
case type CT704 = shape(...)|noreturn;
              type AliasCT704 = CT704;

  
class CheckAliasCT704<T as AliasCT704> extends BaseCheck {
  const type T = AliasCT704;
  const string NAME = 'AliasCT704';

  <<__LateInit>>
  private AliasCT704 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT704 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT704 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT704>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT704>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT704 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT704> {
    return vec[shape('x' => 10),shape()];
  }
}
case type CT705 = string|noreturn;
              type AliasCT705 = CT705;

  
class CheckAliasCT705<T as AliasCT705> extends BaseCheck {
  const type T = AliasCT705;
  const string NAME = 'AliasCT705';

  <<__LateInit>>
  private AliasCT705 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT705 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT705 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT705>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT705>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT705 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT705> {
    return vec['','hello world'];
  }
}
case type CT706 = vec<mixed>|noreturn;
              type AliasCT706 = CT706;

  
class CheckAliasCT706<T as AliasCT706> extends BaseCheck {
  const type T = AliasCT706;
  const string NAME = 'AliasCT706';

  <<__LateInit>>
  private AliasCT706 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT706 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT706 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT706>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT706>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT706 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT706> {
    return vec[vec[]];
  }
}
case type CT707 = vec_or_dict<string>|noreturn;
              type AliasCT707 = CT707;

  
class CheckAliasCT707<T as AliasCT707> extends BaseCheck {
  const type T = AliasCT707;
  const string NAME = 'AliasCT707';

  <<__LateInit>>
  private AliasCT707 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT707 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT707 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT707>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT707>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT707 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT707> {
    return vec[dict[],vec[]];
  }
}
case type CT708 = void|noreturn;
              type AliasCT708 = CT708;

  
class CheckAliasCT708<T as AliasCT708> extends BaseCheck {
  const type T = AliasCT708;
  const string NAME = 'AliasCT708';

  <<__LateInit>>
  private AliasCT708 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT708 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT708 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT708>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT708>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT708 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT708> {
    return vec[null];
  }
}
case type CT709 = nothing|nothing;
              type AliasCT709 = CT709;

  
class CheckAliasCT709<T as AliasCT709> extends BaseCheck {
  const type T = AliasCT709;
  const string NAME = 'AliasCT709';

  <<__LateInit>>
  private AliasCT709 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT709 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT709 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT709>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT709>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT709 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT709> {
    return vec[];
  }
}
case type CT710 = null|nothing;
              type AliasCT710 = CT710;

  
class CheckAliasCT710<T as AliasCT710> extends BaseCheck {
  const type T = AliasCT710;
  const string NAME = 'AliasCT710';

  <<__LateInit>>
  private AliasCT710 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT710 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT710 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT710>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT710>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT710 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT710> {
    return vec[null];
  }
}
case type CT711 = num|nothing;
              type AliasCT711 = CT711;

  
class CheckAliasCT711<T as AliasCT711> extends BaseCheck {
  const type T = AliasCT711;
  const string NAME = 'AliasCT711';

  <<__LateInit>>
  private AliasCT711 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT711 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT711 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT711>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT711>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT711 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT711> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT712 = resource|nothing;
              type AliasCT712 = CT712;

  
class CheckAliasCT712<T as AliasCT712> extends BaseCheck {
  const type T = AliasCT712;
  const string NAME = 'AliasCT712';

  <<__LateInit>>
  private AliasCT712 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT712 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT712 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT712>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT712>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT712 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT712> {
    return vec[imagecreate(10, 10)];
  }
}
case type CT713 = shape(...)|nothing;
              type AliasCT713 = CT713;

  
class CheckAliasCT713<T as AliasCT713> extends BaseCheck {
  const type T = AliasCT713;
  const string NAME = 'AliasCT713';

  <<__LateInit>>
  private AliasCT713 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT713 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT713 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT713>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT713>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT713 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT713> {
    return vec[shape('x' => 10),shape()];
  }
}
case type CT714 = string|nothing;
              type AliasCT714 = CT714;

  
class CheckAliasCT714<T as AliasCT714> extends BaseCheck {
  const type T = AliasCT714;
  const string NAME = 'AliasCT714';

  <<__LateInit>>
  private AliasCT714 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT714 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT714 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT714>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT714>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT714 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT714> {
    return vec['','hello world'];
  }
}
case type CT715 = vec<mixed>|nothing;
              type AliasCT715 = CT715;

  
class CheckAliasCT715<T as AliasCT715> extends BaseCheck {
  const type T = AliasCT715;
  const string NAME = 'AliasCT715';

  <<__LateInit>>
  private AliasCT715 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT715 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT715 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT715>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT715>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT715 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT715> {
    return vec[vec[]];
  }
}
case type CT716 = vec_or_dict<string>|nothing;
              type AliasCT716 = CT716;

  
class CheckAliasCT716<T as AliasCT716> extends BaseCheck {
  const type T = AliasCT716;
  const string NAME = 'AliasCT716';

  <<__LateInit>>
  private AliasCT716 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT716 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT716 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT716>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT716>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT716 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT716> {
    return vec[dict[],vec[]];
  }
}
case type CT717 = void|nothing;
              type AliasCT717 = CT717;

  
class CheckAliasCT717<T as AliasCT717> extends BaseCheck {
  const type T = AliasCT717;
  const string NAME = 'AliasCT717';

  <<__LateInit>>
  private AliasCT717 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT717 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT717 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT717>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT717>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT717 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT717> {
    return vec[null];
  }
}
case type CT718 = null|null;
              type AliasCT718 = CT718;

  
class CheckAliasCT718<T as AliasCT718> extends BaseCheck {
  const type T = AliasCT718;
  const string NAME = 'AliasCT718';

  <<__LateInit>>
  private AliasCT718 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT718 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT718 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT718>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT718>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT718 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT718> {
    return vec[null];
  }
}
case type CT719 = num|null;
              type AliasCT719 = CT719;

  
class CheckAliasCT719<T as AliasCT719> extends BaseCheck {
  const type T = AliasCT719;
  const string NAME = 'AliasCT719';

  <<__LateInit>>
  private AliasCT719 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT719 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT719 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT719>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT719>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT719 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT719> {
    return vec[0,0.0,1,3.14,null];
  }
}
case type CT720 = resource|null;
              type AliasCT720 = CT720;

  
class CheckAliasCT720<T as AliasCT720> extends BaseCheck {
  const type T = AliasCT720;
  const string NAME = 'AliasCT720';

  <<__LateInit>>
  private AliasCT720 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT720 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT720 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT720>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT720>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT720 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT720> {
    return vec[imagecreate(10, 10),null];
  }
}
case type CT721 = shape(...)|null;
              type AliasCT721 = CT721;

  
class CheckAliasCT721<T as AliasCT721> extends BaseCheck {
  const type T = AliasCT721;
  const string NAME = 'AliasCT721';

  <<__LateInit>>
  private AliasCT721 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT721 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT721 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT721>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT721>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT721 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT721> {
    return vec[null,shape('x' => 10),shape()];
  }
}
case type CT722 = string|null;
              type AliasCT722 = CT722;

  
class CheckAliasCT722<T as AliasCT722> extends BaseCheck {
  const type T = AliasCT722;
  const string NAME = 'AliasCT722';

  <<__LateInit>>
  private AliasCT722 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT722 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT722 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT722>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT722>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT722 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT722> {
    return vec['','hello world',null];
  }
}
case type CT723 = vec<mixed>|null;
              type AliasCT723 = CT723;

  
class CheckAliasCT723<T as AliasCT723> extends BaseCheck {
  const type T = AliasCT723;
  const string NAME = 'AliasCT723';

  <<__LateInit>>
  private AliasCT723 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT723 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT723 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT723>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT723>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT723 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT723> {
    return vec[null,vec[]];
  }
}
case type CT724 = vec_or_dict<string>|null;
              type AliasCT724 = CT724;

  
class CheckAliasCT724<T as AliasCT724> extends BaseCheck {
  const type T = AliasCT724;
  const string NAME = 'AliasCT724';

  <<__LateInit>>
  private AliasCT724 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT724 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT724 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT724>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT724>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT724 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT724> {
    return vec[dict[],null,vec[]];
  }
}
case type CT725 = void|null;
              type AliasCT725 = CT725;

  
class CheckAliasCT725<T as AliasCT725> extends BaseCheck {
  const type T = AliasCT725;
  const string NAME = 'AliasCT725';

  <<__LateInit>>
  private AliasCT725 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT725 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT725 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT725>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT725>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT725 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT725> {
    return vec[null];
  }
}
case type CT726 = num|num;
              type AliasCT726 = CT726;

  
class CheckAliasCT726<T as AliasCT726> extends BaseCheck {
  const type T = AliasCT726;
  const string NAME = 'AliasCT726';

  <<__LateInit>>
  private AliasCT726 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT726 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT726 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT726>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT726>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT726 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT726> {
    return vec[0,0.0,1,3.14];
  }
}
case type CT727 = resource|num;
              type AliasCT727 = CT727;

  
class CheckAliasCT727<T as AliasCT727> extends BaseCheck {
  const type T = AliasCT727;
  const string NAME = 'AliasCT727';

  <<__LateInit>>
  private AliasCT727 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT727 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT727 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT727>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT727>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT727 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT727> {
    return vec[0,0.0,1,3.14,imagecreate(10, 10)];
  }
}
case type CT728 = shape(...)|num;
              type AliasCT728 = CT728;

  
class CheckAliasCT728<T as AliasCT728> extends BaseCheck {
  const type T = AliasCT728;
  const string NAME = 'AliasCT728';

  <<__LateInit>>
  private AliasCT728 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT728 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT728 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT728>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT728>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT728 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT728> {
    return vec[0,0.0,1,3.14,shape('x' => 10),shape()];
  }
}
case type CT729 = string|num;
              type AliasCT729 = CT729;

  
class CheckAliasCT729<T as AliasCT729> extends BaseCheck {
  const type T = AliasCT729;
  const string NAME = 'AliasCT729';

  <<__LateInit>>
  private AliasCT729 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT729 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT729 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT729>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT729>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT729 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT729> {
    return vec['','hello world',0,0.0,1,3.14];
  }
}
case type CT730 = vec<mixed>|num;
              type AliasCT730 = CT730;

  
class CheckAliasCT730<T as AliasCT730> extends BaseCheck {
  const type T = AliasCT730;
  const string NAME = 'AliasCT730';

  <<__LateInit>>
  private AliasCT730 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT730 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT730 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT730>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT730>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT730 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT730> {
    return vec[0,0.0,1,3.14,vec[]];
  }
}
case type CT731 = vec_or_dict<string>|num;
              type AliasCT731 = CT731;

  
class CheckAliasCT731<T as AliasCT731> extends BaseCheck {
  const type T = AliasCT731;
  const string NAME = 'AliasCT731';

  <<__LateInit>>
  private AliasCT731 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT731 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT731 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT731>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT731>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT731 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT731> {
    return vec[0,0.0,1,3.14,dict[],vec[]];
  }
}
case type CT732 = void|num;
              type AliasCT732 = CT732;

  
class CheckAliasCT732<T as AliasCT732> extends BaseCheck {
  const type T = AliasCT732;
  const string NAME = 'AliasCT732';

  <<__LateInit>>
  private AliasCT732 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT732 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT732 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT732>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT732>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT732 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT732> {
    return vec[0,0.0,1,3.14,null];
  }
}
case type CT733 = resource|resource;
              type AliasCT733 = CT733;

  
class CheckAliasCT733<T as AliasCT733> extends BaseCheck {
  const type T = AliasCT733;
  const string NAME = 'AliasCT733';

  <<__LateInit>>
  private AliasCT733 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT733 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT733 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT733>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT733>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT733 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT733> {
    return vec[imagecreate(10, 10)];
  }
}
case type CT734 = shape(...)|resource;
              type AliasCT734 = CT734;

  
class CheckAliasCT734<T as AliasCT734> extends BaseCheck {
  const type T = AliasCT734;
  const string NAME = 'AliasCT734';

  <<__LateInit>>
  private AliasCT734 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT734 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT734 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT734>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT734>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT734 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT734> {
    return vec[imagecreate(10, 10),shape('x' => 10),shape()];
  }
}
case type CT735 = string|resource;
              type AliasCT735 = CT735;

  
class CheckAliasCT735<T as AliasCT735> extends BaseCheck {
  const type T = AliasCT735;
  const string NAME = 'AliasCT735';

  <<__LateInit>>
  private AliasCT735 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT735 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT735 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT735>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT735>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT735 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT735> {
    return vec['','hello world',imagecreate(10, 10)];
  }
}
case type CT736 = vec<mixed>|resource;
              type AliasCT736 = CT736;

  
class CheckAliasCT736<T as AliasCT736> extends BaseCheck {
  const type T = AliasCT736;
  const string NAME = 'AliasCT736';

  <<__LateInit>>
  private AliasCT736 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT736 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT736 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT736>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT736>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT736 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT736> {
    return vec[imagecreate(10, 10),vec[]];
  }
}
case type CT737 = vec_or_dict<string>|resource;
              type AliasCT737 = CT737;

  
class CheckAliasCT737<T as AliasCT737> extends BaseCheck {
  const type T = AliasCT737;
  const string NAME = 'AliasCT737';

  <<__LateInit>>
  private AliasCT737 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT737 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT737 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT737>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT737>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT737 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT737> {
    return vec[dict[],imagecreate(10, 10),vec[]];
  }
}
case type CT738 = void|resource;
              type AliasCT738 = CT738;

  
class CheckAliasCT738<T as AliasCT738> extends BaseCheck {
  const type T = AliasCT738;
  const string NAME = 'AliasCT738';

  <<__LateInit>>
  private AliasCT738 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT738 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT738 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT738>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT738>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT738 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT738> {
    return vec[imagecreate(10, 10),null];
  }
}
case type CT739 = shape(...)|shape(...);
              type AliasCT739 = CT739;

  
class CheckAliasCT739<T as AliasCT739> extends BaseCheck {
  const type T = AliasCT739;
  const string NAME = 'AliasCT739';

  <<__LateInit>>
  private AliasCT739 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT739 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT739 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT739>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT739>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT739 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT739> {
    return vec[shape('x' => 10),shape()];
  }
}
case type CT740 = string|shape(...);
              type AliasCT740 = CT740;

  
class CheckAliasCT740<T as AliasCT740> extends BaseCheck {
  const type T = AliasCT740;
  const string NAME = 'AliasCT740';

  <<__LateInit>>
  private AliasCT740 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT740 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT740 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT740>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT740>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT740 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT740> {
    return vec['','hello world',shape('x' => 10),shape()];
  }
}
case type CT741 = vec<mixed>|shape(...);
              type AliasCT741 = CT741;

  
class CheckAliasCT741<T as AliasCT741> extends BaseCheck {
  const type T = AliasCT741;
  const string NAME = 'AliasCT741';

  <<__LateInit>>
  private AliasCT741 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT741 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT741 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT741>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT741>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT741 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT741> {
    return vec[shape('x' => 10),shape(),vec[]];
  }
}
case type CT742 = vec_or_dict<string>|shape(...);
              type AliasCT742 = CT742;

  
class CheckAliasCT742<T as AliasCT742> extends BaseCheck {
  const type T = AliasCT742;
  const string NAME = 'AliasCT742';

  <<__LateInit>>
  private AliasCT742 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT742 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT742 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT742>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT742>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT742 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT742> {
    return vec[dict[],shape('x' => 10),shape(),vec[]];
  }
}
case type CT743 = void|shape(...);
              type AliasCT743 = CT743;

  
class CheckAliasCT743<T as AliasCT743> extends BaseCheck {
  const type T = AliasCT743;
  const string NAME = 'AliasCT743';

  <<__LateInit>>
  private AliasCT743 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT743 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT743 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT743>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT743>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT743 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT743> {
    return vec[null,shape('x' => 10),shape()];
  }
}
case type CT744 = string|string;
              type AliasCT744 = CT744;

  
class CheckAliasCT744<T as AliasCT744> extends BaseCheck {
  const type T = AliasCT744;
  const string NAME = 'AliasCT744';

  <<__LateInit>>
  private AliasCT744 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT744 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT744 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT744>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT744>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT744 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT744> {
    return vec['','hello world'];
  }
}
case type CT745 = vec<mixed>|string;
              type AliasCT745 = CT745;

  
class CheckAliasCT745<T as AliasCT745> extends BaseCheck {
  const type T = AliasCT745;
  const string NAME = 'AliasCT745';

  <<__LateInit>>
  private AliasCT745 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT745 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT745 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT745>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT745>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT745 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT745> {
    return vec['','hello world',vec[]];
  }
}
case type CT746 = vec_or_dict<string>|string;
              type AliasCT746 = CT746;

  
class CheckAliasCT746<T as AliasCT746> extends BaseCheck {
  const type T = AliasCT746;
  const string NAME = 'AliasCT746';

  <<__LateInit>>
  private AliasCT746 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT746 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT746 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT746>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT746>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT746 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT746> {
    return vec['','hello world',dict[],vec[]];
  }
}
case type CT747 = void|string;
              type AliasCT747 = CT747;

  
class CheckAliasCT747<T as AliasCT747> extends BaseCheck {
  const type T = AliasCT747;
  const string NAME = 'AliasCT747';

  <<__LateInit>>
  private AliasCT747 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT747 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT747 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT747>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT747>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT747 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT747> {
    return vec['','hello world',null];
  }
}
case type CT748 = vec<mixed>|vec<mixed>;
              type AliasCT748 = CT748;

  
class CheckAliasCT748<T as AliasCT748> extends BaseCheck {
  const type T = AliasCT748;
  const string NAME = 'AliasCT748';

  <<__LateInit>>
  private AliasCT748 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT748 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT748 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT748>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT748>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT748 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT748> {
    return vec[vec[]];
  }
}
case type CT749 = vec_or_dict<string>|vec<mixed>;
              type AliasCT749 = CT749;

  
class CheckAliasCT749<T as AliasCT749> extends BaseCheck {
  const type T = AliasCT749;
  const string NAME = 'AliasCT749';

  <<__LateInit>>
  private AliasCT749 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT749 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT749 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT749>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT749>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT749 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT749> {
    return vec[dict[],vec[]];
  }
}
case type CT750 = void|vec<mixed>;
              type AliasCT750 = CT750;

  
class CheckAliasCT750<T as AliasCT750> extends BaseCheck {
  const type T = AliasCT750;
  const string NAME = 'AliasCT750';

  <<__LateInit>>
  private AliasCT750 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT750 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT750 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT750>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT750>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT750 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT750> {
    return vec[null,vec[]];
  }
}
case type CT751 = vec_or_dict<string>|vec_or_dict<string>;
              type AliasCT751 = CT751;

  
class CheckAliasCT751<T as AliasCT751> extends BaseCheck {
  const type T = AliasCT751;
  const string NAME = 'AliasCT751';

  <<__LateInit>>
  private AliasCT751 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT751 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT751 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT751>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT751>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT751 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT751> {
    return vec[dict[],vec[]];
  }
}
case type CT752 = void|vec_or_dict<string>;
              type AliasCT752 = CT752;

  
class CheckAliasCT752<T as AliasCT752> extends BaseCheck {
  const type T = AliasCT752;
  const string NAME = 'AliasCT752';

  <<__LateInit>>
  private AliasCT752 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT752 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT752 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT752>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT752>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT752 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT752> {
    return vec[dict[],null,vec[]];
  }
}
case type CT753 = void|void;
              type AliasCT753 = CT753;

  
class CheckAliasCT753<T as AliasCT753> extends BaseCheck {
  const type T = AliasCT753;
  const string NAME = 'AliasCT753';

  <<__LateInit>>
  private AliasCT753 $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(AliasCT753 $c): void {}

  protected static function funcReturn(mixed $c): AliasCT753 {
    return $c;
  }

  protected static function funcGenericParam<Tx as AliasCT753>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as AliasCT753>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(AliasCT753 $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<AliasCT753> {
    return vec[null];
  }
}

  <<__EntryPoint>>
  function main(): void {
    CheckAliasCT0::run();
CheckAliasCT1::run();
CheckAliasCT2::run();
CheckAliasCT3::run();
CheckAliasCT4::run();
CheckAliasCT5::run();
CheckAliasCT6::run();
CheckAliasCT7::run();
CheckAliasCT8::run();
CheckAliasCT9::run();
CheckAliasCT10::run();
CheckAliasCT11::run();
CheckAliasCT12::run();
CheckAliasCT13::run();
CheckAliasCT14::run();
CheckAliasCT15::run();
CheckAliasCT16::run();
CheckAliasCT17::run();
CheckAliasCT18::run();
CheckAliasCT19::run();
CheckAliasCT20::run();
CheckAliasCT21::run();
CheckAliasCT22::run();
CheckAliasCT23::run();
CheckAliasCT24::run();
CheckAliasCT25::run();
CheckAliasCT26::run();
CheckAliasCT27::run();
CheckAliasCT28::run();
CheckAliasCT29::run();
CheckAliasCT30::run();
CheckAliasCT31::run();
CheckAliasCT32::run();
CheckAliasCT33::run();
CheckAliasCT34::run();
CheckAliasCT35::run();
CheckAliasCT36::run();
CheckAliasCT37::run();
CheckAliasCT38::run();
CheckAliasCT39::run();
CheckAliasCT40::run();
CheckAliasCT41::run();
CheckAliasCT42::run();
CheckAliasCT43::run();
CheckAliasCT44::run();
CheckAliasCT45::run();
CheckAliasCT46::run();
CheckAliasCT47::run();
CheckAliasCT48::run();
CheckAliasCT49::run();
CheckAliasCT50::run();
CheckAliasCT51::run();
CheckAliasCT52::run();
CheckAliasCT53::run();
CheckAliasCT54::run();
CheckAliasCT55::run();
CheckAliasCT56::run();
CheckAliasCT57::run();
CheckAliasCT58::run();
CheckAliasCT59::run();
CheckAliasCT60::run();
CheckAliasCT61::run();
CheckAliasCT62::run();
CheckAliasCT63::run();
CheckAliasCT64::run();
CheckAliasCT65::run();
CheckAliasCT66::run();
CheckAliasCT67::run();
CheckAliasCT68::run();
CheckAliasCT69::run();
CheckAliasCT70::run();
CheckAliasCT71::run();
CheckAliasCT72::run();
CheckAliasCT73::run();
CheckAliasCT74::run();
CheckAliasCT75::run();
CheckAliasCT76::run();
CheckAliasCT77::run();
CheckAliasCT78::run();
CheckAliasCT79::run();
CheckAliasCT80::run();
CheckAliasCT81::run();
CheckAliasCT82::run();
CheckAliasCT83::run();
CheckAliasCT84::run();
CheckAliasCT85::run();
CheckAliasCT86::run();
CheckAliasCT87::run();
CheckAliasCT88::run();
CheckAliasCT89::run();
CheckAliasCT90::run();
CheckAliasCT91::run();
CheckAliasCT92::run();
CheckAliasCT93::run();
CheckAliasCT94::run();
CheckAliasCT95::run();
CheckAliasCT96::run();
CheckAliasCT97::run();
CheckAliasCT98::run();
CheckAliasCT99::run();
CheckAliasCT100::run();
CheckAliasCT101::run();
CheckAliasCT102::run();
CheckAliasCT103::run();
CheckAliasCT104::run();
CheckAliasCT105::run();
CheckAliasCT106::run();
CheckAliasCT107::run();
CheckAliasCT108::run();
CheckAliasCT109::run();
CheckAliasCT110::run();
CheckAliasCT111::run();
CheckAliasCT112::run();
CheckAliasCT113::run();
CheckAliasCT114::run();
CheckAliasCT115::run();
CheckAliasCT116::run();
CheckAliasCT117::run();
CheckAliasCT118::run();
CheckAliasCT119::run();
CheckAliasCT120::run();
CheckAliasCT121::run();
CheckAliasCT122::run();
CheckAliasCT123::run();
CheckAliasCT124::run();
CheckAliasCT125::run();
CheckAliasCT126::run();
CheckAliasCT127::run();
CheckAliasCT128::run();
CheckAliasCT129::run();
CheckAliasCT130::run();
CheckAliasCT131::run();
CheckAliasCT132::run();
CheckAliasCT133::run();
CheckAliasCT134::run();
CheckAliasCT135::run();
CheckAliasCT136::run();
CheckAliasCT137::run();
CheckAliasCT138::run();
CheckAliasCT139::run();
CheckAliasCT140::run();
CheckAliasCT141::run();
CheckAliasCT142::run();
CheckAliasCT143::run();
CheckAliasCT144::run();
CheckAliasCT145::run();
CheckAliasCT146::run();
CheckAliasCT147::run();
CheckAliasCT148::run();
CheckAliasCT149::run();
CheckAliasCT150::run();
CheckAliasCT151::run();
CheckAliasCT152::run();
CheckAliasCT153::run();
CheckAliasCT154::run();
CheckAliasCT155::run();
CheckAliasCT156::run();
CheckAliasCT157::run();
CheckAliasCT158::run();
CheckAliasCT159::run();
CheckAliasCT160::run();
CheckAliasCT161::run();
CheckAliasCT162::run();
CheckAliasCT163::run();
CheckAliasCT164::run();
CheckAliasCT165::run();
CheckAliasCT166::run();
CheckAliasCT167::run();
CheckAliasCT168::run();
CheckAliasCT169::run();
CheckAliasCT170::run();
CheckAliasCT171::run();
CheckAliasCT172::run();
CheckAliasCT173::run();
CheckAliasCT174::run();
CheckAliasCT175::run();
CheckAliasCT176::run();
CheckAliasCT177::run();
CheckAliasCT178::run();
CheckAliasCT179::run();
CheckAliasCT180::run();
CheckAliasCT181::run();
CheckAliasCT182::run();
CheckAliasCT183::run();
CheckAliasCT184::run();
CheckAliasCT185::run();
CheckAliasCT186::run();
CheckAliasCT187::run();
CheckAliasCT188::run();
CheckAliasCT189::run();
CheckAliasCT190::run();
CheckAliasCT191::run();
CheckAliasCT192::run();
CheckAliasCT193::run();
CheckAliasCT194::run();
CheckAliasCT195::run();
CheckAliasCT196::run();
CheckAliasCT197::run();
CheckAliasCT198::run();
CheckAliasCT199::run();
CheckAliasCT200::run();
CheckAliasCT201::run();
CheckAliasCT202::run();
CheckAliasCT203::run();
CheckAliasCT204::run();
CheckAliasCT205::run();
CheckAliasCT206::run();
CheckAliasCT207::run();
CheckAliasCT208::run();
CheckAliasCT209::run();
CheckAliasCT210::run();
CheckAliasCT211::run();
CheckAliasCT212::run();
CheckAliasCT213::run();
CheckAliasCT214::run();
CheckAliasCT215::run();
CheckAliasCT216::run();
CheckAliasCT217::run();
CheckAliasCT218::run();
CheckAliasCT219::run();
CheckAliasCT220::run();
CheckAliasCT221::run();
CheckAliasCT222::run();
CheckAliasCT223::run();
CheckAliasCT224::run();
CheckAliasCT225::run();
CheckAliasCT226::run();
CheckAliasCT227::run();
CheckAliasCT228::run();
CheckAliasCT229::run();
CheckAliasCT230::run();
CheckAliasCT231::run();
CheckAliasCT232::run();
CheckAliasCT233::run();
CheckAliasCT234::run();
CheckAliasCT235::run();
CheckAliasCT236::run();
CheckAliasCT237::run();
CheckAliasCT238::run();
CheckAliasCT239::run();
CheckAliasCT240::run();
CheckAliasCT241::run();
CheckAliasCT242::run();
CheckAliasCT243::run();
CheckAliasCT244::run();
CheckAliasCT245::run();
CheckAliasCT246::run();
CheckAliasCT247::run();
CheckAliasCT248::run();
CheckAliasCT249::run();
CheckAliasCT250::run();
CheckAliasCT251::run();
CheckAliasCT252::run();
CheckAliasCT253::run();
CheckAliasCT254::run();
CheckAliasCT255::run();
CheckAliasCT256::run();
CheckAliasCT257::run();
CheckAliasCT258::run();
CheckAliasCT259::run();
CheckAliasCT260::run();
CheckAliasCT261::run();
CheckAliasCT262::run();
CheckAliasCT263::run();
CheckAliasCT264::run();
CheckAliasCT265::run();
CheckAliasCT266::run();
CheckAliasCT267::run();
CheckAliasCT268::run();
CheckAliasCT269::run();
CheckAliasCT270::run();
CheckAliasCT271::run();
CheckAliasCT272::run();
CheckAliasCT273::run();
CheckAliasCT274::run();
CheckAliasCT275::run();
CheckAliasCT276::run();
CheckAliasCT277::run();
CheckAliasCT278::run();
CheckAliasCT279::run();
CheckAliasCT280::run();
CheckAliasCT281::run();
CheckAliasCT282::run();
CheckAliasCT283::run();
CheckAliasCT284::run();
CheckAliasCT285::run();
CheckAliasCT286::run();
CheckAliasCT287::run();
CheckAliasCT288::run();
CheckAliasCT289::run();
CheckAliasCT290::run();
CheckAliasCT291::run();
CheckAliasCT292::run();
CheckAliasCT293::run();
CheckAliasCT294::run();
CheckAliasCT295::run();
CheckAliasCT296::run();
CheckAliasCT297::run();
CheckAliasCT298::run();
CheckAliasCT299::run();
CheckAliasCT300::run();
CheckAliasCT301::run();
CheckAliasCT302::run();
CheckAliasCT303::run();
CheckAliasCT304::run();
CheckAliasCT305::run();
CheckAliasCT306::run();
CheckAliasCT307::run();
CheckAliasCT308::run();
CheckAliasCT309::run();
CheckAliasCT310::run();
CheckAliasCT311::run();
CheckAliasCT312::run();
CheckAliasCT313::run();
CheckAliasCT314::run();
CheckAliasCT315::run();
CheckAliasCT316::run();
CheckAliasCT317::run();
CheckAliasCT318::run();
CheckAliasCT319::run();
CheckAliasCT320::run();
CheckAliasCT321::run();
CheckAliasCT322::run();
CheckAliasCT323::run();
CheckAliasCT324::run();
CheckAliasCT325::run();
CheckAliasCT326::run();
CheckAliasCT327::run();
CheckAliasCT328::run();
CheckAliasCT329::run();
CheckAliasCT330::run();
CheckAliasCT331::run();
CheckAliasCT332::run();
CheckAliasCT333::run();
CheckAliasCT334::run();
CheckAliasCT335::run();
CheckAliasCT336::run();
CheckAliasCT337::run();
CheckAliasCT338::run();
CheckAliasCT339::run();
CheckAliasCT340::run();
CheckAliasCT341::run();
CheckAliasCT342::run();
CheckAliasCT343::run();
CheckAliasCT344::run();
CheckAliasCT345::run();
CheckAliasCT346::run();
CheckAliasCT347::run();
CheckAliasCT348::run();
CheckAliasCT349::run();
CheckAliasCT350::run();
CheckAliasCT351::run();
CheckAliasCT352::run();
CheckAliasCT353::run();
CheckAliasCT354::run();
CheckAliasCT355::run();
CheckAliasCT356::run();
CheckAliasCT357::run();
CheckAliasCT358::run();
CheckAliasCT359::run();
CheckAliasCT360::run();
CheckAliasCT361::run();
CheckAliasCT362::run();
CheckAliasCT363::run();
CheckAliasCT364::run();
CheckAliasCT365::run();
CheckAliasCT366::run();
CheckAliasCT367::run();
CheckAliasCT368::run();
CheckAliasCT369::run();
CheckAliasCT370::run();
CheckAliasCT371::run();
CheckAliasCT372::run();
CheckAliasCT373::run();
CheckAliasCT374::run();
CheckAliasCT375::run();
CheckAliasCT376::run();
CheckAliasCT377::run();
CheckAliasCT378::run();
CheckAliasCT379::run();
CheckAliasCT380::run();
CheckAliasCT381::run();
CheckAliasCT382::run();
CheckAliasCT383::run();
CheckAliasCT384::run();
CheckAliasCT385::run();
CheckAliasCT386::run();
CheckAliasCT387::run();
CheckAliasCT388::run();
CheckAliasCT389::run();
CheckAliasCT390::run();
CheckAliasCT391::run();
CheckAliasCT392::run();
CheckAliasCT393::run();
CheckAliasCT394::run();
CheckAliasCT395::run();
CheckAliasCT396::run();
CheckAliasCT397::run();
CheckAliasCT398::run();
CheckAliasCT399::run();
CheckAliasCT400::run();
CheckAliasCT401::run();
CheckAliasCT402::run();
CheckAliasCT403::run();
CheckAliasCT404::run();
CheckAliasCT405::run();
CheckAliasCT406::run();
CheckAliasCT407::run();
CheckAliasCT408::run();
CheckAliasCT409::run();
CheckAliasCT410::run();
CheckAliasCT411::run();
CheckAliasCT412::run();
CheckAliasCT413::run();
CheckAliasCT414::run();
CheckAliasCT415::run();
CheckAliasCT416::run();
CheckAliasCT417::run();
CheckAliasCT418::run();
CheckAliasCT419::run();
CheckAliasCT420::run();
CheckAliasCT421::run();
CheckAliasCT422::run();
CheckAliasCT423::run();
CheckAliasCT424::run();
CheckAliasCT425::run();
CheckAliasCT426::run();
CheckAliasCT427::run();
CheckAliasCT428::run();
CheckAliasCT429::run();
CheckAliasCT430::run();
CheckAliasCT431::run();
CheckAliasCT432::run();
CheckAliasCT433::run();
CheckAliasCT434::run();
CheckAliasCT435::run();
CheckAliasCT436::run();
CheckAliasCT437::run();
CheckAliasCT438::run();
CheckAliasCT439::run();
CheckAliasCT440::run();
CheckAliasCT441::run();
CheckAliasCT442::run();
CheckAliasCT443::run();
CheckAliasCT444::run();
CheckAliasCT445::run();
CheckAliasCT446::run();
CheckAliasCT447::run();
CheckAliasCT448::run();
CheckAliasCT449::run();
CheckAliasCT450::run();
CheckAliasCT451::run();
CheckAliasCT452::run();
CheckAliasCT453::run();
CheckAliasCT454::run();
CheckAliasCT455::run();
CheckAliasCT456::run();
CheckAliasCT457::run();
CheckAliasCT458::run();
CheckAliasCT459::run();
CheckAliasCT460::run();
CheckAliasCT461::run();
CheckAliasCT462::run();
CheckAliasCT463::run();
CheckAliasCT464::run();
CheckAliasCT465::run();
CheckAliasCT466::run();
CheckAliasCT467::run();
CheckAliasCT468::run();
CheckAliasCT469::run();
CheckAliasCT470::run();
CheckAliasCT471::run();
CheckAliasCT472::run();
CheckAliasCT473::run();
CheckAliasCT474::run();
CheckAliasCT475::run();
CheckAliasCT476::run();
CheckAliasCT477::run();
CheckAliasCT478::run();
CheckAliasCT479::run();
CheckAliasCT480::run();
CheckAliasCT481::run();
CheckAliasCT482::run();
CheckAliasCT483::run();
CheckAliasCT484::run();
CheckAliasCT485::run();
CheckAliasCT486::run();
CheckAliasCT487::run();
CheckAliasCT488::run();
CheckAliasCT489::run();
CheckAliasCT490::run();
CheckAliasCT491::run();
CheckAliasCT492::run();
CheckAliasCT493::run();
CheckAliasCT494::run();
CheckAliasCT495::run();
CheckAliasCT496::run();
CheckAliasCT497::run();
CheckAliasCT498::run();
CheckAliasCT499::run();
CheckAliasCT500::run();
CheckAliasCT501::run();
CheckAliasCT502::run();
CheckAliasCT503::run();
CheckAliasCT504::run();
CheckAliasCT505::run();
CheckAliasCT506::run();
CheckAliasCT507::run();
CheckAliasCT508::run();
CheckAliasCT509::run();
CheckAliasCT510::run();
CheckAliasCT511::run();
CheckAliasCT512::run();
CheckAliasCT513::run();
CheckAliasCT514::run();
CheckAliasCT515::run();
CheckAliasCT516::run();
CheckAliasCT517::run();
CheckAliasCT518::run();
CheckAliasCT519::run();
CheckAliasCT520::run();
CheckAliasCT521::run();
CheckAliasCT522::run();
CheckAliasCT523::run();
CheckAliasCT524::run();
CheckAliasCT525::run();
CheckAliasCT526::run();
CheckAliasCT527::run();
CheckAliasCT528::run();
CheckAliasCT529::run();
CheckAliasCT530::run();
CheckAliasCT531::run();
CheckAliasCT532::run();
CheckAliasCT533::run();
CheckAliasCT534::run();
CheckAliasCT535::run();
CheckAliasCT536::run();
CheckAliasCT537::run();
CheckAliasCT538::run();
CheckAliasCT539::run();
CheckAliasCT540::run();
CheckAliasCT541::run();
CheckAliasCT542::run();
CheckAliasCT543::run();
CheckAliasCT544::run();
CheckAliasCT545::run();
CheckAliasCT546::run();
CheckAliasCT547::run();
CheckAliasCT548::run();
CheckAliasCT549::run();
CheckAliasCT550::run();
CheckAliasCT551::run();
CheckAliasCT552::run();
CheckAliasCT553::run();
CheckAliasCT554::run();
CheckAliasCT555::run();
CheckAliasCT556::run();
CheckAliasCT557::run();
CheckAliasCT558::run();
CheckAliasCT559::run();
CheckAliasCT560::run();
CheckAliasCT561::run();
CheckAliasCT562::run();
CheckAliasCT563::run();
CheckAliasCT564::run();
CheckAliasCT565::run();
CheckAliasCT566::run();
CheckAliasCT567::run();
CheckAliasCT568::run();
CheckAliasCT569::run();
CheckAliasCT570::run();
CheckAliasCT571::run();
CheckAliasCT572::run();
CheckAliasCT573::run();
CheckAliasCT574::run();
CheckAliasCT575::run();
CheckAliasCT576::run();
CheckAliasCT577::run();
CheckAliasCT578::run();
CheckAliasCT579::run();
CheckAliasCT580::run();
CheckAliasCT581::run();
CheckAliasCT582::run();
CheckAliasCT583::run();
CheckAliasCT584::run();
CheckAliasCT585::run();
CheckAliasCT586::run();
CheckAliasCT587::run();
CheckAliasCT588::run();
CheckAliasCT589::run();
CheckAliasCT590::run();
CheckAliasCT591::run();
CheckAliasCT592::run();
CheckAliasCT593::run();
CheckAliasCT594::run();
CheckAliasCT595::run();
CheckAliasCT596::run();
CheckAliasCT597::run();
CheckAliasCT598::run();
CheckAliasCT599::run();
CheckAliasCT600::run();
CheckAliasCT601::run();
CheckAliasCT602::run();
CheckAliasCT603::run();
CheckAliasCT604::run();
CheckAliasCT605::run();
CheckAliasCT606::run();
CheckAliasCT607::run();
CheckAliasCT608::run();
CheckAliasCT609::run();
CheckAliasCT610::run();
CheckAliasCT611::run();
CheckAliasCT612::run();
CheckAliasCT613::run();
CheckAliasCT614::run();
CheckAliasCT615::run();
CheckAliasCT616::run();
CheckAliasCT617::run();
CheckAliasCT618::run();
CheckAliasCT619::run();
CheckAliasCT620::run();
CheckAliasCT621::run();
CheckAliasCT622::run();
CheckAliasCT623::run();
CheckAliasCT624::run();
CheckAliasCT625::run();
CheckAliasCT626::run();
CheckAliasCT627::run();
CheckAliasCT628::run();
CheckAliasCT629::run();
CheckAliasCT630::run();
CheckAliasCT631::run();
CheckAliasCT632::run();
CheckAliasCT633::run();
CheckAliasCT634::run();
CheckAliasCT635::run();
CheckAliasCT636::run();
CheckAliasCT637::run();
CheckAliasCT638::run();
CheckAliasCT639::run();
CheckAliasCT640::run();
CheckAliasCT641::run();
CheckAliasCT642::run();
CheckAliasCT643::run();
CheckAliasCT644::run();
CheckAliasCT645::run();
CheckAliasCT646::run();
CheckAliasCT647::run();
CheckAliasCT648::run();
CheckAliasCT649::run();
CheckAliasCT650::run();
CheckAliasCT651::run();
CheckAliasCT652::run();
CheckAliasCT653::run();
CheckAliasCT654::run();
CheckAliasCT655::run();
CheckAliasCT656::run();
CheckAliasCT657::run();
CheckAliasCT658::run();
CheckAliasCT659::run();
CheckAliasCT660::run();
CheckAliasCT661::run();
CheckAliasCT662::run();
CheckAliasCT663::run();
CheckAliasCT664::run();
CheckAliasCT665::run();
CheckAliasCT666::run();
CheckAliasCT667::run();
CheckAliasCT668::run();
CheckAliasCT669::run();
CheckAliasCT670::run();
CheckAliasCT671::run();
CheckAliasCT672::run();
CheckAliasCT673::run();
CheckAliasCT674::run();
CheckAliasCT675::run();
CheckAliasCT676::run();
CheckAliasCT677::run();
CheckAliasCT678::run();
CheckAliasCT679::run();
CheckAliasCT680::run();
CheckAliasCT681::run();
CheckAliasCT682::run();
CheckAliasCT683::run();
CheckAliasCT684::run();
CheckAliasCT685::run();
CheckAliasCT686::run();
CheckAliasCT687::run();
CheckAliasCT688::run();
CheckAliasCT689::run();
CheckAliasCT690::run();
CheckAliasCT691::run();
CheckAliasCT692::run();
CheckAliasCT693::run();
CheckAliasCT694::run();
CheckAliasCT695::run();
CheckAliasCT696::run();
CheckAliasCT697::run();
CheckAliasCT698::run();
CheckAliasCT699::run();
CheckAliasCT700::run();
CheckAliasCT701::run();
CheckAliasCT702::run();
CheckAliasCT703::run();
CheckAliasCT704::run();
CheckAliasCT705::run();
CheckAliasCT706::run();
CheckAliasCT707::run();
CheckAliasCT708::run();
CheckAliasCT709::run();
CheckAliasCT710::run();
CheckAliasCT711::run();
CheckAliasCT712::run();
CheckAliasCT713::run();
CheckAliasCT714::run();
CheckAliasCT715::run();
CheckAliasCT716::run();
CheckAliasCT717::run();
CheckAliasCT718::run();
CheckAliasCT719::run();
CheckAliasCT720::run();
CheckAliasCT721::run();
CheckAliasCT722::run();
CheckAliasCT723::run();
CheckAliasCT724::run();
CheckAliasCT725::run();
CheckAliasCT726::run();
CheckAliasCT727::run();
CheckAliasCT728::run();
CheckAliasCT729::run();
CheckAliasCT730::run();
CheckAliasCT731::run();
CheckAliasCT732::run();
CheckAliasCT733::run();
CheckAliasCT734::run();
CheckAliasCT735::run();
CheckAliasCT736::run();
CheckAliasCT737::run();
CheckAliasCT738::run();
CheckAliasCT739::run();
CheckAliasCT740::run();
CheckAliasCT741::run();
CheckAliasCT742::run();
CheckAliasCT743::run();
CheckAliasCT744::run();
CheckAliasCT745::run();
CheckAliasCT746::run();
CheckAliasCT747::run();
CheckAliasCT748::run();
CheckAliasCT749::run();
CheckAliasCT750::run();
CheckAliasCT751::run();
CheckAliasCT752::run();
CheckAliasCT753::run();
  }