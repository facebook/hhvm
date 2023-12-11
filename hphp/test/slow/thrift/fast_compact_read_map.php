<?hh

abstract class ThriftData {
  public $data = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class DictI64 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::MAP,
      'ktype' => TType::I64,
      'vtype' => TType::I64,
      'key' => dict['type' => TType::I64],
      'val' => dict['type' => TType::I64],
    ],
  ];
}

class DictI08 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::MAP,
      'ktype' => TType::I08,
      'vtype' => TType::I08,
      'key' => dict['type' => TType::I08],
      'val' => dict['type' => TType::I08],
    ],
  ];
}

class MapI64 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'collection',
      'type' => TType::MAP,
      'ktype' => TType::I64,
      'vtype' => TType::I64,
      'key' => dict['type' => TType::I64],
      'val' => dict['type' => TType::I64],
    ],
  ];
}

class MapI08 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'collection',
      'type' => TType::MAP,
      'ktype' => TType::I08,
      'vtype' => TType::I08,
      'key' => dict['type' => TType::I08],
      'val' => dict['type' => TType::I08],
    ],
  ];
}

function test($name, $classname, $data) {
  $obj = new $classname();
  $obj->data = $data;

  echo "---- $name ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'foo', 2, $obj, 20);
  var_dump(thrift_protocol_read_compact($p, $classname));
}

function main(): mixed {
  test("empty dict", DictI64::class, dict[]);
  test("fast decoded DictI64", DictI64::class, dict(vec[0x7FFFFFFFFFFFFF00, 0x7FFFFFFFFFFFFF01]));
  test("fast decoded DictI08", DictI08::class, dict(vec[100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110]));

  test("empty map", MapI64::class, new Map(null));
  test("fast decoded MapI64", MapI64::class, new Map(Vector{0x7FFFFFFFFFFFFF00, 0x7FFFFFFFFFFFFF01}));
  test("fast decoded MapI08", MapI08::class, new Map(Vector{100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110}));
}

<<__EntryPoint>>
function main_collections() :mixed{
  require 'common.inc';
  main();
}
