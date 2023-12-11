<?hh

class TestStruct {
  const SPEC = dict[
    -1 => dict[
      'var' => 'aBool',
      'type' => TType::BOOL,
      'is_terse' => true,
    ],
    1 => dict[
      'var' => 'anInt',
      'type' => TType::I32,
      'is_terse' => true,
    ],
    2 => dict[
      'var' => 'aString',
      'type' => TType::STRING,
      'is_terse' => true,
    ],
    3 => dict[
      'var' => 'aDouble',
      'type' => TType::DOUBLE,
      'is_terse' => true,
    ],
    4 => dict[
      'var' => 'anInt64',
      'type' => TType::I64,
      'is_terse' => true,
    ],
    5 => dict[
      'var' => 'aList',
      'type' => TType::LST,
      'etype' => TType::DOUBLE,
      'elem' => dict[
        'type' => TType::DOUBLE,
      ],
      'is_terse' => true,
    ],
    6 => dict[
      'var' => 'aMap',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'is_terse' => true,
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
      'is_terse' => true,
      'elem' => dict[
        'type' => TType::I32,
      ],
    ],
    8 => dict[
      'var' => 'anByte',
      'type' => TType::BYTE,
      'is_terse' => true,
    ],
    9 => dict[
      'var' => 'anI16',
      'type' => TType::I16,
      'is_terse' => true,
    ],
    10 => dict[
      'var' => 'anI64CustomDefault',
      'type' => TType::I64,
      'is_terse' => true,
    ]
  ];
  public $aBool = false;
  public $anInt = 0;
  public $aString = '';
  public $aDouble = 0.0;
  public $anInt64 = 0;
  public $aList = vec[];
  public $aMap = dict[];
  public $aSet = dict[];
  public $anByte = 0;
  public $anI16 = 0;
  public $anI64CustomDefault = 7;

  public function __construct($vals=null)[] {}

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {
    $this->aBool = false;
    $this->anInt = 0;
    $this->aString = '';
    $this->aDouble = 0.0;
    $this->anInt64 = 0;
    $this->aList = vec[];
    $this->aMap = dict[];
    $this->aSet = dict[];
    $this->anByte = 0;
    $this->anI16 = 0;
    $this->anI64CustomDefault = 0;
  }
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
  var_dump($v1);
  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);
  var_dump(md5($p->getTransport()->buff));
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));

  // Terse struct
  $v1 = new TestStruct();
  $v1->aBool = true;
  $v1->anInt = 0;
  $v1->aString = '';
  $v1->aDouble = 0.0;
  $v1->anInt64 = 0;
  $v1->aList = vec[];
  $v1->aMap = dict[];
  $v1->aSet = dict[];
  $v1->anByte = 0;
  $v1->anI16 = 0;
  $v1->anI64CustomDefault = 0;

  echo "---- terse: compact ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact($p, 'foo', 2, $v1, 20);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));

  echo "---- terse: binary ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'foo', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));
}

<<__EntryPoint>>
function main_1556() :mixed{
  require 'common.inc';
  test();
}
