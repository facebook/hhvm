<?hh

class DummyTransport {
  public $buff = '';
  public $pos = 0;

  function write($buff) { $this->buff .= $buff; }
  function flush() {}
  function read($n) {
    $r = substr($this->buff, $this->pos, $n);
    $this->pos += strlen($r);
    return $r;
  }
}

class DummyProtocol {
  public $t;
  function __construct() { $this->t = new DummyTransport(); }
  function getTransport() { return $this->t; }
}

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

class TestStruct {
  const SPEC = darray[
    5 => darray[
      'var' => 'aList',
      'format' => 'collection',
      'type' => TType::LST,
      'etype' => TType::DOUBLE,
      'elem' => darray[ 'type' => TType::DOUBLE ],
    ],
    6 => darray[
      'var' => 'aMap',
      'format' => 'collection',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::DOUBLE,
      'key' => darray[ 'type' => TType::I32 ],
      'val' => darray[ 'type' => TType::DOUBLE ],
    ],
    7 => darray[
      'var' => 'aSet',
      'format' => 'collection',
      'type' => TType::SET,
      'etype' => TType::I32,
      'elem' => darray[ 'type' => TType::I32 ],
    ],
  ];

  public $aList = null;
  public $aMap = null;
  public $aSet = null;
}

function test($name, $list, $map, $set) {
  $s = new TestStruct();
  $s->aList = $list;
  $s->aMap = $map;
  $s->aSet = $set;

  echo "---- $name: compact ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact($p, 'foo', 1, $s, 20);
  $p->getTransport()->buff[1] = pack('C', 0x42);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));

  echo "---- $name: binary ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'foo', 1, $s, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));
}

function main() {
  test(
    "real collections",
    Vector {13.3, 23.4, 3576.2},
    Map {10 => 1.2, 43 => 5.33},
    Set {10, 11}
  );
  test(
    "hack arrays",
    vec[13.3, 23.4, 3576.2],
    dict[10 => 1.2, 43 => 5.33],
    keyset[10, 11]
  );
  test(
    "php arrays",
    varray[13.3, 23.4, 3576.2],
    darray[10 => 1.2, 43 => 5.33],
    darray[10 => 'doesnt', 11 => 'matter']
  );
  $listObj = new stdClass();
  $listObj->{0} = 13.3;
  $listObj->{1} = 23.4;
  $listObj->{2} = 3576.2;
  $mapObj = new stdClass();
  $mapObj->{10} = 1.2;
  $mapObj->{43} = 5.33;
  $setObj = new stdClass();
  $setObj->{10} = 'doesnt';
  $setObj->{11} = 'matter';
  test("sadly, this is legal", $listObj, $mapObj, $setObj);
}

<<__EntryPoint>>
function main_collections() {
main();
}
