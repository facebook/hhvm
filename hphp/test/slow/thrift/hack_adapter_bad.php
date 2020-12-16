<?hh

class Struct1 {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
      'adapter' => 'ThisAdapterDoesntExist',
    ],
  ];
  public $value = null;
  public function __construct($value) { $this->value = $value; }
}

class BadAdapter {}
class Struct2 {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I32,
      'adapter' => BadAdapter::class,
    ],
  ];
  public $value = null;
  public function __construct($value) { $this->value = $value; }
}

function testBinary() {
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

function testCompact() {
  $p = new DummyProtocol();
  try {
    thrift_protocol_write_compact($p, 'foomethod', 2, new Struct1(42), 20);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    thrift_protocol_write_compact($p, 'foomethod', 2, new Struct2(42), 20);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main_forward_compatibility() {
  require 'common.inc';
  echo "--- binary ---\n";
  testBinary();
  echo "--- compact ---\n";
  testCompact();
}
