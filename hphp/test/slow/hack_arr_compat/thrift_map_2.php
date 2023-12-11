<?hh

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
  const LIST    = 15;
  const UTF8   = 16;
  const UTF16  = 17;
}

class DummyProtocol {
  public $t;
  function __construct() {
    $this->t = new DummyTransport();
  }
  function getTransport() :mixed{
    return $this->t;
  }
}

class DummyTransport {
  public $buff = '';
  public $pos = 0;
  function flush() :mixed{
 }
  function onewayFlush() :mixed{}
  function write($buff) :mixed{
    $this->buff .= $buff;
  }
  function read($n) :mixed{
    $r = substr($this->buff, $this->pos, $n);
    $this->pos += $n;
    return $r;
  }
}

class Mappish {
  const darray<int, darray<string, mixed>> SPEC = dict[
    1 => dict[
      'var' => 'extraData',
      'type' => TType::MAP,
      'ktype' => TType::STRING,
      'vtype' => TType::STRING,
      'key' => dict[
        'type' => TType::STRING,
      ],
      'val' => dict[
        'type' => TType::STRING,
        ],
        'format' => 'array',
      ],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class Settish {
  const darray<int, darray<string, mixed>> SPEC = dict[
    1 => dict[
      'var' => 'extraData',
      'type' => TType::SET,
      'etype' => TType::STRING,
      'elem' => dict[
        'type' => TType::STRING,
      ],
      'format' => 'array',
    ],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class TestStruct {
  const SPEC = dict[
    0 => dict[
      'var' => 'success',
      'type' => TType::STRUCT,
      'class' => 'Config'
    ],
  ];

  const int STRUCTURAL_ID = 957977401221134810;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

function test() :mixed{
  //var_dump(TestStruct::SPEC);
  $p = new DummyProtocol();
  $v1 = new Mappish();
  $v1->extraData = Map{"1" => 2};
  var_dump($v1);
  thrift_protocol_write_binary($p, 'foomethod', 2, $v1, 20, true);
  var_dump(md5($p->getTransport()->buff));
  $mappish_obj = thrift_protocol_read_binary($p, 'Mappish', true);
  var_dump($mappish_obj);
  var_dump(is_darray($mappish_obj->extraData));

  $p = new DummyProtocol();
  var_dump($v1);
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v1, 20, true);
  var_dump(md5($p->getTransport()->buff));
  $mappish_obj = thrift_protocol_read_compact($p, 'Mappish');
  var_dump($mappish_obj);
  var_dump(is_darray($mappish_obj->extraData));

  echo "-------\n\n";

  $p = new DummyProtocol();
  $v1 = new Settish();
  $v1->extraData = dict["1" => true];
  var_dump($v1);
  thrift_protocol_write_binary($p, 'foomethod', 2, $v1, 20, true);
  var_dump(md5($p->getTransport()->buff));
  $settish_obj = thrift_protocol_read_binary($p, 'Settish', true);
  var_dump($settish_obj);
  var_dump(is_darray($settish_obj->extraData));

  $p = new DummyProtocol();
  var_dump($v1);
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v1, 20, true);
  var_dump(md5($p->getTransport()->buff));
  $settish_obj = thrift_protocol_read_compact($p, 'Settish');
  var_dump($settish_obj);
  var_dump(is_darray($settish_obj->extraData));
}

<<__EntryPoint>>
function main_1556() :mixed{
test();
}
