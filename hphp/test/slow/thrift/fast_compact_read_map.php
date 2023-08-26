<?hh

class DictI64 {
  const SPEC = darray[
    1 => darray[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::MAP,
      'ktype' => TType::I64,
      'vtype' => TType::I64,
      'key' => darray['type' => TType::I64],
      'val' => darray['type' => TType::I64],
    ],
  ];

  public $data = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class DictI08 {
  const SPEC = darray[
    1 => darray[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::MAP,
      'ktype' => TType::I08,
      'vtype' => TType::I08,
      'key' => darray['type' => TType::I08],
      'val' => darray['type' => TType::I08],
    ],
  ];

  public $data = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
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
  test("fast decoded DictI64", DictI64::class, dict[1 => 0x7FFFFFFFFFFFFF00, 2 => 0x7FFFFFFFFFFFFF01]);
  test("fast decoded DictI08", DictI08::class, dict[0 => 100, 1 => 101, 2 => 102, 3 => 103, 4 => 104, 5 => 105, 6 => 106, 7 => 107, 8 => 108, 9 => 109, 10 => 110]);
}

<<__EntryPoint>>
function main_collections() :mixed{
  require 'common.inc';
  main();
}
