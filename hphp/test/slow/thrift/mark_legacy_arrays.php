<?hh

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
    public varray<varray<string>> $varray = vec[],
    public darray<string, darray<string, string>> $darray = dict[],
    public darray<string, bool> $darraySet = dict[],
    public varray<TestStruct> $structVarray = vec[],
  )[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function struct(): TestStruct {
  $struct = new TestStruct();
  $struct->varray = vec[vec['foo']];
  $struct->darray = dict['foo' => dict['bar' => 'baz']];
  $struct->darraySet = dict['foo' => true];
  $struct->structVarray = vec[new TestStruct()];
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

function test_thrift_protocol_read_binary() :mixed{
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

function test_thrift_protocol_read_compact() :mixed{
  echo "== thrift_protocol_read_compact ==\n";

  echo "original struct\n";
  log_markings(struct());

  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'foo', 2, struct(), 20);
  $struct = thrift_protocol_read_compact(
    $p,
    TestStruct::class,
  );
  echo "deserialized\n";
  log_markings($struct);

  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'foo', 2, struct(), 20);
  $struct = thrift_protocol_read_compact(
    $p,
    TestStruct::class,
    THRIFT_MARK_LEGACY_ARRAYS,
  );
  echo "deserialized w/ flag\n";
  log_markings($struct);
}

<<__EntryPoint>>
function main() :mixed{
  require 'common.inc';
  test_thrift_protocol_read_binary();
  test_thrift_protocol_read_compact();
}
