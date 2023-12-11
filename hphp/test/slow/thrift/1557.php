<?hh

class TestStruct {
  const SPEC = dict[
    -1 => dict[
      'var' => 'aBool',
      'type' => TType::BOOL,
    ],
    1 => dict[
      'var' => 'anInt',
      'type' => TType::I32,
    ],
    2 => dict[
      'var' => 'aDouble',
      'type' => TType::DOUBLE,
    ],
    3 => dict[
      'var' => 'anInt64',
      'type' => TType::I64,
    ],
    4 => dict[
      'var' => 'anByte',
      'type' => TType::BYTE,
    ],
    5 => dict[
      'var' => 'anI16',
      'type' => TType::I16,
    ],
    6 => dict[
      'var' => 'aFloat',
      'type' => TType::FLOAT,
    ],
    7 => dict[
      'var' => 'bFloat',
      'type' => TType::FLOAT,
    ],
  ];
  public $aBool = null;
  public $anInt = null;
  public $aDouble = null;
  public $anInt64 = null;
  public $anByte = null;
  public $anI16 = null;
  public $aFloat = null;
  public $bFloat = null;
  public function __construct($vals=null)[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}
function test() :mixed{
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->aBool = false;
  $v1->anInt = -1234;
  $v1->aDouble = -1.2345;
  $v1->anInt64 = -1;
  $v1->anByte = -12;
  $v1->anI16 = -123;
  $v1->aFloat = 1.25;
  $v1->bFloat = 3.14159265358979323846264;
  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));
}

<<__EntryPoint>>
function main_1557() :mixed{
  require 'common.inc';
  test();
}
