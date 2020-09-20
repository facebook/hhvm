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
  function putBack($s) {}
}

class TestStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'aString',
      'type' => TType::STRING,
    ],
  ];
  public $aString = null;
  public function __construct($vals=null) {}
}

function test() {
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->aString = str_repeat('x', 1000000);
  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);
  $p->getTransport()->buff = str_replace(
    pack('N', 1000000),
    pack('N', (1<<32) - 2),
    $p->getTransport()->buff);
  thrift_protocol_read_binary($p, 'TestStruct', true);
}

<<__EntryPoint>>
function main_thrift() {
test();
print "Done\n";
}
