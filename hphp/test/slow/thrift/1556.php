<?hh

class TestStruct {
  const SPEC = darray[
    -1 => darray[
      'var' => 'aBool',
      'type' => TType::BOOL,
      'is_terse' => true,
    ],
    1 => darray[
      'var' => 'anInt',
      'type' => TType::I32,
      'is_terse' => true,
    ],
    2 => darray[
      'var' => 'aString',
      'type' => TType::STRING,
      'is_terse' => true,
    ],
    3 => darray[
      'var' => 'aDouble',
      'type' => TType::DOUBLE,
      'is_terse' => true,
    ],
    4 => darray[
      'var' => 'anInt64',
      'type' => TType::I64,
      'is_terse' => true,
    ],
    5 => darray[
      'var' => 'aList',
      'type' => TType::LST,
      'etype' => TType::DOUBLE,
      'elem' => darray[
        'type' => TType::DOUBLE,
      ],
      'is_terse' => true,
    ],
    6 => darray[
      'var' => 'aMap',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'is_terse' => true,
      'vtype' => TType::DOUBLE,
      'key' => darray[
        'type' => TType::I32,
      ],
      'val' => darray[
        'type' => TType::DOUBLE,
      ],
    ],
    7 => darray[
      'var' => 'aSet',
      'type' => TType::SET,
      'etype' => TType::I32,
      'is_terse' => true,
      'elem' => darray[
        'type' => TType::I32,
      ],
    ],
    8 => darray[
      'var' => 'anByte',
      'type' => TType::BYTE,
      'is_terse' => true,
    ],
    9 => darray[
      'var' => 'anI16',
      'type' => TType::I16,
      'is_terse' => true,
    ],
    10 => darray[
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
  public $aList = varray[];
  public $aMap = darray[];
  public $aSet = darray[];
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
    $this->aList = varray[];
    $this->aMap = darray[];
    $this->aSet = darray[];
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
  $v1->aList = varray[13.3, 23.4, 3576.2];
  $v1->aMap = darray[10=>1.2, 43=>5.33];
  $v1->aSet = darray[10=>true, 11=>true];
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
  $v1->aList = varray[];
  $v1->aMap = darray[];
  $v1->aSet = darray[];
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
