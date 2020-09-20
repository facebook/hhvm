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
  const LST    = 15;
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
  function flush() {
 }
  function write($buff) {
    $this->buff .= $buff;
  }
  function read($n) {
    $r = substr($this->buff, $this->pos, $n);
    $this->pos += $n;
    return $r;
  }
}
class TestStruct {
  const SPEC = darray[
    -1 => darray[
      'var' => 'aBool',
      'type' => TType::BOOL,
    ],
    1 => darray[
      'var' => 'anInt',
      'type' => TType::I32,
    ],
    2 => darray[
      'var' => 'aDouble',
      'type' => TType::DOUBLE,
    ],
    3 => darray[
      'var' => 'anInt64',
      'type' => TType::I64,
    ],
    4 => darray[
      'var' => 'anByte',
      'type' => TType::BYTE,
    ],
    5 => darray[
      'var' => 'anI16',
      'type' => TType::I16,
    ],
    6 => darray[
      'var' => 'aFloat',
      'type' => TType::FLOAT,
    ],
    7 => darray[
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
  public function __construct($vals=null) {}
}
function test() {
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
function main_1557() {
test();
}
