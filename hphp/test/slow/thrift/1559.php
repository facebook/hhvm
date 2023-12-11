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
      'var' => 'aString',
      'type' => TType::STRING,
    ],
    3 => dict[
      'var' => 'aDouble',
      'type' => TType::DOUBLE,
    ],
    4 => dict[
      'var' => 'anInt64',
      'type' => TType::I64,
    ],
    5 => dict[
      'var' => 'aList',
      'type' => TType::LST,
      'etype' => TType::DOUBLE,
      'elem' => dict[
        'type' => TType::DOUBLE,
      ],
    ],
    6 => dict[
      'var' => 'aMap',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::DOUBLE,
      'key' => dict[
        'type' => TType::I32,
      ],
      'val' => dict[
        'type' => TType::DOUBLE,
      ],
    ],
    7 => dict[
      'var' => 'aSet',
      'type' => TType::SET,
      'etype' => TType::I32,
      'elem' => dict[
        'type' => TType::I32,
      ],
    ],
    8 => dict[
      'var' => 'anByte',
      'type' => TType::BYTE,
    ],
    9 => dict[
      'var' => 'anI16',
      'type' => TType::I16,
    ],
    10 => dict[
      'var' => 'aFloat',
      'type' => TType::FLOAT,
    ],
    11 => dict[
      'var' => 'bFloat',
      'type' => TType::FLOAT,
    ],
  ];
  public $aBool = null;
  public $anInt = null;
  public $aString = null;
  public $aDouble = null;
  public $anInt64 = null;
  public $aList = null;
  public $aMap = null;
  public $aSet = null;
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
  $v1->aBool = true;
  $v1->anInt = 1234;
  $v1->aString = 'abcdef';
  $v1->aDouble = 1.2345;
  $v1->anInt64 = 8589934592;
  $v1->aList = vec[13.3, 23.4, 3576.2];
  $v1->aMap = dict[10=>1.2, 43=>5.33];
  $v1->aSet = dict[10=>true, 11=>true];
  $v1->anByte = 123;
  $v1->anI16 = 1234;
  $v1->aFloat = 1.25;
  $v1->bFloat = 3.14159265358979323846264;
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v1, 20);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));
}

<<__EntryPoint>>
function main_1559() :mixed{
  require 'common.inc';
  test();
}
