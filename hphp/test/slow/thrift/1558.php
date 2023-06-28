<?hh

class TestStruct {
  const SPEC = darray[
    -1 => darray[
      'var' => 'aBool',
      'type' => TType::BOOL
    ],
    1 => darray[
      'var' => 'anInt',
      'type' => TType::I32,
    ],
    2 => darray[
      'var' => 'aString',
      'type' => TType::STRING,
    ],
    3 => darray[
      'var' => 'aDouble',
      'type' => TType::DOUBLE,
    ],
    4 => darray[
      'var' => 'anInt64',
      'type' => TType::I64,
    ],
    5 => darray[
      'var' => 'aList',
      'type' => TType::LST,
      'etype' => TType::DOUBLE,
      'elem' => darray[
        'type' => TType::DOUBLE,
      ],
    ],
    6 => darray[
      'var' => 'aMap',
      'type' => TType::MAP,
      'ktype' => TType::I32,
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
      'elem' => darray[
        'type' => TType::I32,
      ],
    ],
    8 => darray[
      'var' => 'anByte',
      'type' => TType::BYTE,
    ],
    9 => darray[
      'var' => 'anI16',
      'type' => TType::I16,
    ],
    10 => darray[
      'var' => 'aFloat',
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
  $v1->aList = varray[13.3, 23.4, 3576.2];
  $v1->aMap = darray[10=>1.2, 43=>5.33];
  $v1->aSet = darray[10=>true, 11=>true];
  $v1->anByte = 123;
  $v1->anI16 = 1234;
  $v1->aFloat = 1.25;
  thrift_protocol_write_compact2($p, 'foomethod', 1, $v1, 20);
  var_dump(md5($p->getTransport()->buff));
}

<<__EntryPoint>>
function main_1558() :mixed{
  require 'common.inc';
  test();
}
