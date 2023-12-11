<?hh

class Struct1 {
  const SPEC = dict[
    1 => dict[
      'var' => 'value',
      'type' => \TType::I32,
      'adapter' => 'ThisAdapterDoesntExist',
    ],
  ];
  public $value = null;
  public function __construct($value)[] { $this->value = $value; }
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class BadAdapter {}
class Struct2 {
  const SPEC = dict[
    1 => dict[
      'var' => 'value',
      'type' => \TType::I32,
      'adapter' => BadAdapter::class,
    ],
  ];
  public $value = null;
  public function __construct($value)[] { $this->value = $value; }
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function testBinary() :mixed{
  $p = new DummyProtocol();
  try {
    thrift_protocol_write_binary($p, 'foomethod', 1, new Struct1(42), 20, true);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    thrift_protocol_write_binary($p, 'foomethod', 1, new Struct2(42), 20, true);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

function testCompact() :mixed{
  $p = new DummyProtocol();
  try {
    thrift_protocol_write_compact2($p, 'foomethod', 2, new Struct1(42), 20);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    thrift_protocol_write_compact2($p, 'foomethod', 2, new Struct2(42), 20);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main_forward_compatibility() :mixed{
  require 'common.inc';
  echo "--- binary ---\n";
  testBinary();
  echo "--- compact ---\n";
  testCompact();
}
