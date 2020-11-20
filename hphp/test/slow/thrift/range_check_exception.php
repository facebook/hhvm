<?hh

class TestStruct {
  public $anI32 = null;

  const SPEC = darray[
    1 => darray[
      'var' => 'anI32',
      'type' => TType::I32,
    ],
  ];
}

<<__EntryPoint>>
function test() {
  require 'common.inc';
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->anI32 = 1 << 31;
  try {
    thrift_protocol_write_compact($p, 'foomethod', 1, $v1, 20, true);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}
