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

class Settish {
  const darray<int, darray<string, mixed>> SPEC = dict[
    1 => dict[
      'var' => 'extraData',
      'type' => TType::SET,
      'etype' => TType::STRING,
      'format' => 'collection',
      'elem' => dict[
          'type' => TType::STRING,
      ],
    ],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

function test() :mixed{
  $p = new DummyProtocol();
  $v1 = new Settish();
  $v1->extraData = Set{"1"};
  var_dump($v1);
  thrift_protocol_write_binary($p, 'foomethod', 2, $v1, 20, true);
  var_dump(md5($p->getTransport()->buff));
  var_dump(thrift_protocol_read_binary($p, 'Settish', true));
}

<<__EntryPoint>>
function main_1556() :mixed{
test();
}
