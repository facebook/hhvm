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

  const SPEC = dict[
    1 => shape(
      'var' => 'varray',
      'type' => TType::LST,
      'etype' => TType::LST,
      'elem' => shape(
        'type' => TType::LST,
        'etype' => TType::STRING,
        'elem' => shape(
          'type' => TType::STRING,
        ),
        'format' => 'array',
      ),
      'format' => 'array',
    ),
    2 => shape(
      'var' => 'darray',
      'type' => TType::MAP,
      'ktype' => TType::STRING,
      'vtype' => TType::MAP,
      'key' => shape(
        'type' => TType::STRING,
      ),
      'val' => shape(
        'type' => TType::MAP,
        'ktype' => TType::STRING,
        'vtype' => TType::STRING,
        'key' => shape(
          'type' => TType::STRING,
        ),
        'val' => shape(
          'type' => TType::STRING,
        ),
        'format' => 'array',
      ),
      'format' => 'array',
    ),
    3 => shape(
      'var' => 'darraySet',
      'type' => TType::SET,
      'etype' => TType::STRING,
      'elem' => shape(
        'type' => TType::STRING,
      ),
      'format' => 'array',
    ),
    4 => shape(
      'var' => 'structVarray',
      'type' => TType::LST,
      'etype' => TType::STRUCT,
      'elem' => shape(
        'type' => TType::STRUCT,
        'class' => TestStruct::class,
      ),
      'format' => 'array',
    ),
  ];

  public function __construct(
    public varray<varray<string>> $varray = varray[],
    public darray<string, darray<string, string>> $darray = darray[],
    public darray<string, bool> $darraySet = darray[],
    public varray<TestStruct> $structVarray = varray[],
  ) {}
}

function struct(): TestStruct {
  $struct = new TestStruct();
  $struct->varray = varray[varray['foo']];
  $struct->darray = darray['foo' => darray['bar' => 'baz']];
  $struct->darraySet = darray['foo' => true];
  $struct->structVarray = varray[new TestStruct()];
  return $struct;
}

function log_markings(TestStruct $struct): void {
  $cases = dict[
    'varray' => $struct->varray,
    'varray[0]' => $struct->varray[0],
    'darray' => $struct->darray,
    'darray[\'foo\']' => $struct->darray['foo'],
    'darraySet' => $struct->darraySet,
    'structVarray[0]->varray' => $struct->structVarray[0]->varray,
  ];
  foreach ($cases as $name => $value) {
    echo sprintf(
      "  %s: %s\n",
      $name,
      HH\is_array_marked_legacy($value) ? 'true' : 'false',
    );
  }
}

function test_thrift_protocol_read_binary() {
  echo "== thrift_protocol_read_binary ==\n";

  echo "original struct\n";
  log_markings(struct());

  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'foo', 2, struct(), 20, true);
  $struct = thrift_protocol_read_binary(
    $p,
    TestStruct::class,
    true,
  );
  echo "deserialized\n";
  log_markings($struct);

  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'foo', 2, struct(), 20, true);
  $struct = thrift_protocol_read_binary(
    $p,
    TestStruct::class,
    true,
    THRIFT_MARK_LEGACY_ARRAYS,
  );
  echo "deserialized w/ flag\n";
  log_markings($struct);
}

function test_thrift_protocol_read_compact() {
  echo "== thrift_protocol_read_compact ==\n";

  echo "original struct\n";
  log_markings(struct());

  $p = new DummyProtocol();
  thrift_protocol_write_compact($p, 'foo', 1, struct(), 20);
  $p->getTransport()->buff[1] = pack('C', 0x42);
  $struct = thrift_protocol_read_compact(
    $p,
    TestStruct::class,
  );
  echo "deserialized\n";
  log_markings($struct);

  $p = new DummyProtocol();
  thrift_protocol_write_compact($p, 'foo', 1, struct(), 20);
  $p->getTransport()->buff[1] = pack('C', 0x42);
  $struct = thrift_protocol_read_compact(
    $p,
    TestStruct::class,
    THRIFT_MARK_LEGACY_ARRAYS,
  );
  echo "deserialized w/ flag\n";
  log_markings($struct);
}

<<__EntryPoint>>
function main() {
  test_thrift_protocol_read_binary();
  test_thrift_protocol_read_compact();
}
