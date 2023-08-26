<?hh

class ListI64 {
  const SPEC = darray[
    1 => darray[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::LST,
      'etype' => TType::I64,
      'elem' => darray[ 'type' => TType::I64 ],
    ],
  ];

  public $data = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class ListI08 {
  const SPEC = darray[
    1 => darray[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::LST,
      'etype' => TType::I08,
      'elem' => darray[ 'type' => TType::I08 ],
    ],
  ];

  public $data = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

function test($name, $classname, $data) :mixed{
  $obj = new $classname();
  $obj->data = $data;

  echo "---- $name ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'foo', 2, $obj, 20);
  var_dump(thrift_protocol_read_compact($p, $classname));
}

function main(): mixed {
  test("empty vec", ListI64::class, vec[]);
  test("fast decoded vec", ListI64::class, vec[0x7FFFFFFFFFFFFF00, 0x7FFFFFFFFFFFFF01]);
  test("fast decoded vec", ListI08::class, vec[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);
}

<<__EntryPoint>>
function main_collections() :mixed{
  require 'common.inc';
  main();
}
