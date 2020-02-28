<?hh

class TType {
  const I16    = 6;
  const I32    = 8;
  const MAP    = 13;
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
  public $anI16 = null;
  public $map = null;

  const SPEC = darray[
    1 => darray[
      'var' => 'anI32',
      'type' => TType::I32,
    ],
    2 => darray[
      'var' => 'anI16',
      'type' => TType::I16,
    ],
    3 => darray[
      'var' => 'map',
      'type' => TType::MAP,
      'ktype' => TType::I16,
      'vtype' => TType::I16,
      'key' => darray[
        'type' => TType::I16,
      ],
      'val' => darray[
        'type' => TType::I16,
      ],
    ],
  ];
}

<<__EntryPoint>>
function test() {
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->anI32 = (1 << 31) - 1;
  $v1->anI16 = (1 << 15) - 1;
  $v1->anI32 = 1 << 31;
  $v1->anI16 = 1 << 15;
  $v1->map = darray[(1 << 15) => 0];
  thrift_protocol_write_compact($p, 'foomethod', 1, $v1, 20, true);
}
