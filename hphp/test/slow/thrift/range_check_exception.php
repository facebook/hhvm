<?hh

class TestStruct {
  public $anI32 = null;

  const SPEC = dict[
    1 => dict[
      'var' => 'anI32',
      'type' => TType::I32,
    ],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

<<__EntryPoint>>
function test() :mixed{
  require 'common.inc';
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->anI32 = 1 << 31;
  try {
    thrift_protocol_write_compact2($p, 'foomethod', 1, $v1, 20, true);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}
