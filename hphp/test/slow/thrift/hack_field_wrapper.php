<?hh // partial
interface IThriftStruct {}
abstract class IThriftFieldWrapper<TThriftType, TStruct as IThriftStruct> {
  protected function __construct(
    protected TThriftType $value,
    protected int $fieldId,
    protected TStruct $struct,
  )[] {}

  final public function getValue_DO_NOT_USE_THRIFT_INTERNAL()[]: TThriftType {
    return $this->value;
  }

  final public function setValue_DO_NOT_USE_THRIFT_INTERNAL(
    TThriftType $value,
  )[write_props]: void {
    $this->value = $value;
  }

  final public static function fromThrift_DO_NOT_USE_THRIFT_INTERNAL(
    TThriftType $value,
    int $field_id,
    TStruct $parent,
  )[]: this {
    return new static($value, $field_id, $parent);
  }

  abstract public static function genToThrift(
    this $value,
  ): Awaitable<TThriftType>;

  abstract public static function genFromThrift(
    TThriftType $value,
    int $field_id,
    TStruct $parent,
  ): Awaitable<this>;

  abstract public function genUnwrap(): Awaitable<TThriftType>;
  abstract public function genWrap(TThriftType $value): Awaitable<void>;
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

  public function __construct()[] {}

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function print(): void {
    echo "\t\t ----InnerStruct----\n";
    echo "\t\t\t\t value = ";
    echo $this->value;
    echo "\n";
  }
}

async function getStructWithWrapper() {
  $v = new OuterStructWithWrapper();
  await $v->get_value()->genWrap(42);
  return $v;
}

async function testBinary() {
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
}

async function testCompact() {
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
}

<<__EntryPoint>>
async function main_forward_compatibility() {
  require 'common.inc';
  echo "--- binary ---\n";
  await testBinary();
  echo "--- compact ---\n";
  await testCompact();
}
