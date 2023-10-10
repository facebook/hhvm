<?hh
interface IThriftStruct {}

abstract class IThriftWrapper<TThriftType> {
  protected function __construct(protected TThriftType $value)[] {}

  final public function getValue_DO_NOT_USE_THRIFT_INTERNAL()[]: TThriftType {
    return $this->value;
  }

  final public function setValue_DO_NOT_USE_THRIFT_INTERNAL(
    TThriftType $value,
  )[write_props]: void {
    $this->value = $value;
  }

  final public static function toThrift_DO_NOT_USE_THRIFT_INTERNAL(
    this $wrapped_value,
  )[zoned]: TThriftType {
    return $wrapped_value->value;
  }

  abstract public static function genToThrift(
    this $wrapped_value,
  )[zoned_shallow]: Awaitable<TThriftType>;

  abstract public function genUnwrap()[zoned_shallow]: Awaitable<TThriftType>;
  abstract public function genWrap(
    TThriftType $value,
  )[zoned_local]: Awaitable<void>;
}

abstract class IThriftStructWrapper<TThriftStructType as ?IThriftStruct>
  extends IThriftWrapper<TThriftStructType> {
  protected function __construct(TThriftStructType $value)[] {
    parent::__construct($value);
  }

  final public static function fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
    <<__Explicit>> TThriftType__ as ?IThriftStruct,
  >(TThriftType__ $value)[]: this {
    return new static($value);
  }

  abstract public static function genFromThrift<
    <<__Explicit>> TThriftType__ as ?IThriftStruct,
  >(
    TThriftType__ $value,
  )[zoned]: Awaitable<IThriftStructWrapper<TThriftType__>>;
}

abstract class IThriftFieldWrapper<
  TThriftType,
  TThriftStruct as IThriftStruct,
> extends IThriftWrapper<TThriftType> {
  protected function __construct(
    TThriftType $value,
    protected int $fieldId,
    protected TThriftStruct $struct,
  )[] {
    parent::__construct($value);
  }

  final public static function fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
    <<__Explicit>> TThriftType__,
    <<__Explicit>> TThriftStruct__ as IThriftStruct,
  >(TThriftType__ $value, int $field_id, TThriftStruct__ $parent)[]: this {
    return new static($value, $field_id, $parent);
  }

  abstract public static function genFromThrift<
    <<__Explicit>> TThriftType__,
    <<__Explicit>> TThriftStruct__ as IThriftStruct,
  >(TThriftType__ $value, int $field_id, TThriftStruct__ $parent)[
    zoned_shallow,
  ]: Awaitable<IThriftFieldWrapper<TThriftType__, TThriftStruct__>>;
}

final class MyFieldWrapper<TThriftType, TStruct as IThriftStruct>
  extends IThriftFieldWrapper<TThriftType, TStruct> {
  <<__Override>>
  public static async function genToThrift(
    this $value,
  ): Awaitable<TThriftType> {
    return await $value->genUnwrap();
  }

  <<__Override>>
  public static async function genFromThrift(
    TThriftType $value,
    int $field_id,
    TStruct $parent,
  ): Awaitable<this> {
    return new static($value, $field_id, $parent);
  }

  <<__Override>>
  public async function genUnwrap(): Awaitable<TThriftType> {
    return $this->value;
  }

  <<__Override>>
  public async function genWrap(TThriftType $value): Awaitable<void> {
    $this->value = $value;
  }
}

final class MyStructWrapper<TStruct as IThriftStruct> extends IThriftStructWrapper<TStruct> {

<<__Override>>
  public static async function genToThrift(
    this $value,
  ): Awaitable<TStruct> {
    return await $value->genUnwrap();
  }

  <<__Override>>
  public static async function genFromThrift<<<__Explicit>> TThriftType__ as ?IThriftStruct,
  >(
    TThriftType__ $value,
  )[zoned]: Awaitable<MyStructWrapper<TThriftType__>> {
    return new static($value);
  }

  <<__Override>>
  public async function genUnwrap(): Awaitable<TThriftType> {
    return $this->value;
  }

  <<__Override>>
  public async function genWrap(TThriftType $value): Awaitable<void> {
    $this->value = $value;
  }
}

interface IThriftAdapter {
  abstract const type THackType;
  abstract const type TThriftType;
  public static function toThrift(
    this::THackType $hack_obj,
  )[write_props]: this::TThriftType;
  public static function fromThrift(
    this::TThriftType $thrift_obj,
  )[]: this::THackType;
}

class StringToIntPrimitiveAdapter {
  const type THackType = string;

  public static function toThrift(string $hack_value): int {
    return (int)$hack_value;
  }
  public static function fromThrift(int $thrift_value): string {
    return "Int value is ".$thrift_value;
  }
}

class OuterStructWithWrapperAndAdapter implements IThriftStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
      'is_wrapped' => true,
      'adapter' => \StringToIntPrimitiveAdapter::class,
    ],
    2 => darray[
      'var' => 'struct_value',
      'type' => \TType::STRUCT,
      'is_wrapped' => true,
      'class' => InnerStruct::class,
    ],
  ];
  private ?\MyFieldWrapper<?\MyAdapter::THackType> $value;
  private ?\MyFieldWrapper<?InnerStruct> $struct_value;

  public function get_value(
  )[]: \MyFieldWrapper<?\StringToIntPrimitiveAdapter::THackType> {
    return $this->value as nonnull;
  }

  public function get_struct_value()[]: \MyFieldWrapper<?\InnerStruct> {
    return $this->struct_value as nonnull;
  }

  public function __construct()[] {
    $this->value = \MyFieldWrapper::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
      ?\StringToIntPrimitiveAdapter::THackType,
      OuterStructWithWrapperAndAdapter,
    >(null, 1, $this);
    $this->struct_value =
      \MyFieldWrapper::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
        ?InnerStruct,
        OuterStructWithWrapperAndAdapter,
      >(null, 2, $this);
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}

  public async function print(): Awaitable<void> {
    $inner_struct = await $this->get_struct_value()->genUnwrap();
    echo "----OuterStructWithWrapperAndAdapter----\n";
    echo "\t\t value = ";
    echo await $this->get_value()->genUnwrap();
    echo "\n";
    echo "\t\t struct_value = ";
    echo $inner_struct ? $inner_struct->print() : "null";
    echo "\n";
  }
}

class OuterStructWithWrapper implements IThriftStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
      'is_wrapped' => true,
    ],
    2 => darray[
      'var' => 'struct_value',
      'type' => \TType::STRUCT,
      'class' => InnerStruct::class,
      'is_wrapped' => true,
    ],
  ];
  private ?\MyFieldWrapper<?int> $value;
  private ?\MyFieldWrapper<?InnerStruct> $struct_value;

  public function get_value()[]: \MyFieldWrapper<?int> {
    return $this->value as nonnull;
  }

  public function get_struct_value()[]: \MyFieldWrapper<?\InnerStruct> {
    return $this->struct_value as nonnull;
  }

  public function __construct()[] {
    $this->value = \MyFieldWrapper::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
      ?int,
      OuterStructWithWrapper,
    >(null, 1, $this);
    $this->struct_value =
      \MyFieldWrapper::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
        ?InnerStruct,
        OuterStructWithWrapper,
      >(null, 2, $this);
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}

  public async function print(): Awaitable<void> {
    $inner_struct = await $this->get_struct_value()->genUnwrap();
    echo "----OuterStructWithWrapper----\n";
    echo "\t\t value = ";
    echo await $this->get_value()->genUnwrap();
    echo "\n";
    echo "\t\t struct_value = ";
    echo $inner_struct ? $inner_struct->print() : "null";
    echo "\n";
  }
}

class OuterStructNoWrappedFields {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
    ],
    2 => darray[
      'var' => 'struct_value',
      'type' => \TType::STRUCT,
      'class' => InnerStruct::class,
    ],
  ];
  public ?int $value = null;
  public ?InnerStruct $struct_value;

  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}

  public function print(): void {
    echo "----OuterStructNoWrappedFields----\n";
    echo "\t\t value = ";
    echo $this->value;
    echo "\n";
    echo "\t\t struct_value = ";
    echo $this->struct_value ? $this->struct_value->print() : "null";
    echo "\n";
  }
}

class InnerStruct implements IThriftStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
    ],
  ];
  private ?int $value;

  public function __construct()[] {
    $this->value = 100;
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}

  public function print(): void {
    echo "----InnerStruct----\n";
    echo "\t\t\t\t value = ";
    echo $this->value;
  }
}

type WrappedInnerStruct = MyStructWrapper<InnerStruct>;

class StructWithTypeWrapper  {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
    ],
    2 => darray[
      'var' => 'struct_value',
      'type' => \TType::STRUCT,
      'is_type_wrapped' => true,
      'class' => InnerStruct::class,
    ],
  ];
  public ?int $value = null;
  public ?WrappedInnerStruct $struct_value;

  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function set_struct_value_DO_NOT_USE_THRIFT_INTERNAL(InnerStruct $s)[write_props]:void {
    $this->struct_value = MyStructWrapper::fromThrift_DO_NOT_USE_THRIFT_INTERNAL($s);
  }

  public function clearTerseFields()[write_props]: void {}

  public function print(): void {
    echo "----StructWithTypeWrapper----\n";
    echo "\t\t value = ";
    echo $this->value;
    echo "\n";
    echo "\t\t struct_value = ";
    $val = $this->struct_value?->getValue_DO_NOT_USE_THRIFT_INTERNAL();
    echo $val ? $val->print() : "null";
    echo "\n";
  }
}

async function getStructWithWrapper() :Awaitable<mixed>{
  $v = new OuterStructWithWrapper();
  await $v->get_value()->genWrap(42);
  await $v->get_struct_value()->genWrap(InnerStruct::withDefaultValues());
  return $v;
}

async function testBinary() :Awaitable<mixed>{
  $p = new DummyProtocol();
  $v = await getStructWithWrapper();
  await $v->print();
  thrift_protocol_write_binary($p, 'foomethod', 1, $v, 20, true);
  var_dump(md5($p->getTransport()->buff));
  $new_value = thrift_protocol_read_binary($p, 'OuterStructWithWrapper', true);
  await $new_value->print();
  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value =
    thrift_protocol_read_binary($p, 'OuterStructNoWrappedFields', true);
  $new_value->print();

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value =
    thrift_protocol_read_binary($p, 'OuterStructWithWrapperAndAdapter', true);
  $new_value->print();

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value =
    thrift_protocol_read_binary($p, 'StructWithTypeWrapper', true);
  $new_value->print();
}

async function testCompact() :Awaitable<mixed>{
  $p = new DummyProtocol();
  $v = await getStructWithWrapper();
  $v->print();
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v, 20);
  var_dump(md5($p->getTransport()->buff));
  $new_value = thrift_protocol_read_compact($p, 'OuterStructWithWrapper');
  await $new_value->print();

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value = thrift_protocol_read_compact($p, 'OuterStructNoWrappedFields');
  $new_value->print();

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value =
    thrift_protocol_read_compact($p, 'OuterStructWithWrapperAndAdapter');
  $new_value->print();
  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value =
    thrift_protocol_read_compact($p, 'StructWithTypeWrapper');
  $new_value->print();
}

<<__EntryPoint>>
async function main_forward_compatibility() :Awaitable<mixed>{
  require 'common.inc';
  echo "--- binary ---\n";
  await testBinary();
  echo "--- compact ---\n";
  await testCompact();
}
