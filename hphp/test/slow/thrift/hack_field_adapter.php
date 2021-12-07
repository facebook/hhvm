<?hh // partial
interface IThriftStruct {}
interface IThriftFieldWrapper<TThriftType> {}
interface IThriftFieldAdapter {}

final class MyFieldWrapper<T> implements IThriftFieldWrapper<T>{
  public function __construct(
    public T $value,
    public int $fieldId,
    public IThriftStruct $struct,
  )[] {}

  public function setValue(T $value)[write_props]: void {
    $this->value = $value;
  }

  public function getValue()[]: T {
    return $this->value;
  }
}

final class MyFieldAdapter implements IThriftFieldAdapter{
  public static function fromThrift<TThriftType>(
    TThriftType $thrift_obj,
    int $field_id,
    IThriftStruct $struct,
  )[]: MyFieldWrapper<TThriftType> {
    return new MyFieldWrapper<TThriftType>($thrift_obj, $field_id, $struct);
  }

  public static function toThrift<TThriftType>(
    IThriftFieldWrapper<TThriftType> $hack_obj,
  )[]: TThriftType {
    $hack_obj as MyFieldWrapper<_>;
    return $hack_obj->getValue();
  }

  public static function assign<TThriftType>(
    IThriftFieldWrapper<TThriftType> $hack_obj,
    TThriftType $value,
  )[write_props]: void {
    $hack_obj as MyFieldWrapper<_>;
    $hack_obj->setValue($value);
  }

  public static function assignWrapped<TThriftType>(
    IThriftFieldWrapper<TThriftType> $from,
    IThriftFieldWrapper<TThriftType> $to,
  )[write_props]: void {
    $from as MyFieldWrapper<_>;
    $to as MyFieldWrapper<_>;
    $from->setValue($to->getValue());
  }
}


class OuterStruct implements IThriftStruct{
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
      'field_adapter' => MyFieldAdapter::class,
    ]
  ];
  private ?\MyFieldWrapper<?int> $value;

  public function getWrapped_value()[]: \MyFieldWrapper<int> {
    return $this->value as nonnull;
  }

  public function setWrapped_value(\MyFieldWrapper<?int> $value)[write_props]: void {
    \MyFieldAdapter::assignWrapped<?int>($this->value as nonnull, $value);
   }

  public function get_value()[]: ?int {
    return \MyFieldAdapter::toThrift<?int>($this->value as nonnull);
  }

  public function set_value(?int $value)[write_props]: void {
    \MyFieldAdapter::assign<?int>($this->value as nonnull, $value);
  }

  public function __construct() {
    $this->value = \MyFieldAdapter::fromThrift<?int>(null, 1, $this);
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function print(): void {
    echo "----OuterStruct----\n";
    echo "\t\t value = ";
    echo $this->get_value();
    echo "\n";
  }
}

// This class is identical to OuterStruct but with all the adapters removed.
// It's used to "peek" at the actual serialized data without adapters getting
// in the way.
class OuterStructNoAdapter {
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
    echo "----OuterStructNoAdapter----\n";
    echo "\t\t value = ";
    echo $this->value;
    echo "\n";
  }
}

function getStruct() {
  $v = new OuterStruct();
  $v->set_value(42);
  return $v;
}

function testBinary() {
  $p = new DummyProtocol();
  $v = getStruct();
  $v->print();
  thrift_protocol_write_binary($p, 'foomethod', 1, $v, 20, true);
  var_dump(md5($p->getTransport()->buff));
  $new_value = thrift_protocol_read_binary($p, 'OuterStruct', true);
  $new_value->print();
  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value = thrift_protocol_read_binary($p, 'OuterStructNoAdapter', true);
  $new_value->print();
}

function testCompact() {
  $p = new DummyProtocol();
  $v = getStruct();
  $v->print();
  thrift_protocol_write_compact($p, 'foomethod', 2, $v, 20);
  var_dump(md5($p->getTransport()->buff));
  $new_value = thrift_protocol_read_compact($p, 'OuterStruct');
  $new_value->print();

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  $new_value = thrift_protocol_read_compact($p, 'OuterStructNoAdapter');
  $new_value->print();
}

<<__EntryPoint>>
function main_forward_compatibility() {
  require 'common.inc';
  echo "--- binary ---\n";
  testBinary();
  echo "--- compact ---\n";
  testCompact();
}
