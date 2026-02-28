<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class TProtocolException extends Exception {}

class TType {
  const STOP   = 0;
  const VOID   = 1;
  const BOOL   = 2;
  const BYTE   = 3;
  const I08    = 3;
  const DOUBLE = 4;
  const I16    = 6;
  const I32    = 8;
  const I64    = 10;
  const STRING = 11;
  const UTF7   = 11;
  const STRUCT = 12;
  const MAP    = 13;
  const SET    = 14;
  const LIST   = 15;
  const UTF8   = 16;
  const UTF16  = 17;
  const FLOAT  = 19;
}
class DummyProtocol {
  public $t;
  function __construct() {
    $this->t = new DummyTransport();
  }
  function getTransport() {
    return $this->t;
  }
}
class DummyTransport {
  public $buff = '';
  public $pos = 0;
  function flush() {}
  function onewayFlush() {}
  function putBack($s) {}
  function write($buff) {
    $this->buff .= $buff;
  }
  function read($n) {
    $r = substr($this->buff, $this->pos, $n);
    $this->pos += $n;
    return $r;
  }
}

class DummyStruct {
  const SPEC = vec[];
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}
class DummyStruct2 {
  const SPEC = vec[];
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class TestStruct1 {
  public bool $p1 = true;
  public int $p2 = 123;
  public string $p3 = 'abc';
  public $p4 = 'abc';
  public ?DummyStruct $p5;
  public ?int $p6 = null;

  public function __construct() {
    $this->p5 = new DummyStruct();
  }

  const SPEC = vec[
    dict['var' => 'p1',
         'type' => TType::BOOL],
    dict['var' => 'p2',
         'type' => TType::I32],
    dict['var' => 'p3',
         'type' => TType::STRING],
    dict['var' => 'p4',
         'type' => TType::STRING],
    dict['var' => 'p5',
         'class' => 'DummyStruct',
         'type' => TType::STRUCT],
    dict['var' => 'p6',
         'type' => TType::I32]
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class TestStruct2 {
  public bool $p1 = 123;
  public int $p2 = 'abc';
  public string $p3 = false;
  public $p4 = 'abc';
  public DummyStruct $p5;
  public ?int $p6 = 'abc';

  public function __construct() {
    $this->p5 = new DummyStruct2();
  }

  const SPEC = vec[
    dict['var' => 'p1',
         'type' => TType::I32],
    dict['var' => 'p2',
         'type' => TType::STRING],
    dict['var' => 'p3',
         'type' => TType::BOOL],
    dict['var' => 'p4',
         'type' => TType::STRING],
    dict['var' => 'p5',
         'class' => 'DummyStruct2',
         'type' => TType::STRUCT],
    dict['var' => 'p6',
         'type' => TType::STRING]
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class TestStruct3 {
  public DummyStruct $p1;
  public function __construct() {
    $this->p1 = new DummyStruct();
  }
  const SPEC = vec[
    dict['var' => 'p1',
         'class' => 'DoesntExist',
         'type' => TType::STRUCT]
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

enum ComplexUnionEnum: int {
  _EMPTY_ = 0;
  intValue = 1;
  stringValue = 2;
}

class TestStruct4 {
  const SPEC = vec[
    dict[
      'var' => 'intValue',
      'union' => true,
      'type' => TType::I64,
    ],
    dict[
      'var' => 'stringValue',
      'union' => true,
      'type' => TType::STRING,
    ]
  ];
  public ?int $intValue = 123;
  public ?string $stringValue;
  public ComplexUnionEnum $_type = ComplexUnionEnum::intValue;
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class TestStruct5 {
  const SPEC = vec[
    dict[
      'var' => 'intValue',
      'union' => true,
      'type' => TType::I64,
    ],
    dict[
      'var' => 'stringValue',
      'union' => true,
      'type' => TType::STRING,
    ]
  ];
  public ?int $intValue = 123;
  public ?string $stringValue;
  public string $_type = ComplexUnionEnum::intValue;
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class TestStruct6 {
  public darray $p1 = dict[];
  public varray $p2 = vec[];
  public darray $p3 = dict[];

  public dict $p4 = dict[];
  public vec $p5 = vec[];
  public keyset $p6 = keyset[];

  public Map $p7 = Map{};
  public Vector $p8 = Vector{};
  public Set $p9 = Set{};

  const SPEC = vec[
    dict['var' => 'p1',
         'ktype' => TType::I32,
         'vtype' => TType::STRING,
         'key' => dict['type' => TType::STRING],
         'val' => dict['type' => TType::STRING],
         'type' => TType::MAP],
    dict['var' => 'p2',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'type' => TType::LIST],
    dict['var' => 'p3',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'type' => TType::SET],
    dict['var' => 'p4',
         'ktype' => TType::I32,
         'vtype' => TType::STRING,
         'key' => dict['type' => TType::STRING],
         'val' => dict['type' => TType::STRING],
         'format' => 'harray',
         'type' => TType::MAP],
    dict['var' => 'p5',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'harray',
         'type' => TType::LIST],
    dict['var' => 'p6',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'harray',
         'type' => TType::SET],
    dict['var' => 'p7',
         'ktype' => TType::I32,
         'vtype' => TType::STRING,
         'key' => dict['type' => TType::STRING],
         'val' => dict['type' => TType::STRING],
         'format' => 'collection',
         'type' => TType::MAP],
    dict['var' => 'p8',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'collection',
         'type' => TType::LIST],
    dict['var' => 'p9',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'collection',
         'type' => TType::SET],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class TestStruct7 {
  public dict $p1 = dict[];
  public vec $p2 = vec[];
  public keyset $p3 = keyset[];

  public Map $p4 = Map{};
  public Vector $p5 = Vector{};
  public Set $p6 = Set{};

  public darray $p7 = dict[];
  public varray $p8 = vec[];
  public darray $p9 = dict[];

  const SPEC = vec[
    dict['var' => 'p1',
         'ktype' => TType::I32,
         'vtype' => TType::STRING,
         'key' => dict['type' => TType::STRING],
         'val' => dict['type' => TType::STRING],
         'type' => TType::MAP],
    dict['var' => 'p2',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'type' => TType::LIST],
    dict['var' => 'p3',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'type' => TType::SET],
    dict['var' => 'p4',
         'ktype' => TType::I32,
         'vtype' => TType::STRING,
         'key' => dict['type' => TType::STRING],
         'val' => dict['type' => TType::STRING],
         'format' => 'harray',
         'type' => TType::MAP],
    dict['var' => 'p5',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'harray',
         'type' => TType::LIST],
    dict['var' => 'p6',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'harray',
         'type' => TType::SET],
    dict['var' => 'p7',
         'ktype' => TType::I32,
         'vtype' => TType::STRING,
         'key' => dict['type' => TType::STRING],
         'val' => dict['type' => TType::STRING],
         'format' => 'collection',
         'type' => TType::MAP],
    dict['var' => 'p8',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'collection',
         'type' => TType::LIST],
    dict['var' => 'p9',
         'etype' => TType::I32,
         'elem' => dict['type' => TType::STRING],
         'format' => 'collection',
         'type' => TType::SET],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}
