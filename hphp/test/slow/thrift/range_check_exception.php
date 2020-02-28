<?hh

class TProtocolException extends Exception {}

class TType {
  const I32    = 8;
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
  function onewayFlush() {}
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
  public $anI32 = null;

  const SPEC = darray[
    1 => darray[
      'var' => 'anI32',
      'type' => TType::I32,
    ],
  ];
}

<<__EntryPoint>>
function test() {
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->anI32 = 1 << 31;
  try {
    thrift_protocol_write_compact($p, 'foomethod', 1, $v1, 20, true);
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}
