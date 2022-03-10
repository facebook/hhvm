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

final class MyFieldWrapper<TThriftType, TStruct as IThriftStruct>  extends IThriftFieldWrapper<TThriftType, TStruct>{
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
  ): Awaitable<this>{
    return new static($value,$field_id,$parent);
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

class OuterStruct implements IThriftStruct{
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
      'is_wrapped' => true,
    ]
  ];
  private ?\MyFieldWrapper<?int> $value;

  public function get_value()[]: \MyFieldWrapper<int> {
    return $this->value as nonnull;
  }

  public function __construct()[] {
    $this->value = \MyFieldWrapper::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<?int, OuterStruct>(null, 1, $this);
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public async function print(): Awaitable<void> {
    echo "----OuterStruct----\n";
    echo "\t\t value = ";
    echo await $this->get_value()->genUnwrap();
    echo "\n";
  }
}

// This class is identical to OuterStruct but with all the adapters removed.
// It's used to "peek" at the actual serialized data without adapters getting
// in the way.
class OuterStructNoWrappedFields {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
    ],
  ];
  public ?int $value = null;
  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function print(): void {
    echo "----OuterStructNoWrappedFields----\n";
    echo "\t\t value = ";
    echo $this->value;
    echo "\n";
  }
}

async function getStruct(){
  $v = new OuterStruct();
  await $v->get_value()->genWrap(42);
  return $v;
}

async function testBinary() {
  $p = new DummyProtocol();
  $v = await getStruct();
  $v->print();
  thrift_protocol_write_binary($p, 'foomethod', 1, $v, 20, true);
  var_dump(md5($p->getTransport()->buff));
  $new_value = thrift_protocol_read_binary($p, 'OuterStruct', true);
  await $new_value->print();
  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value = thrift_protocol_read_binary($p, 'OuterStructNoWrappedFields', true);
  $new_value->print();
}

async function testCompact() {
  $p = new DummyProtocol();
  $v = await getStruct();
  $v->print();
  thrift_protocol_write_compact($p, 'foomethod', 2, $v, 20);
  var_dump(md5($p->getTransport()->buff));
  $new_value = thrift_protocol_read_compact($p, 'OuterStruct');
  await $new_value->print();

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value = thrift_protocol_read_compact($p, 'OuterStructNoWrappedFields');
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
