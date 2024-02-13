<?hh

class ThriftData {
  public $data = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class ListI64 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::LST,
      'etype' => TType::I64,
      'elem' => dict[ 'type' => TType::I64 ],
    ],
  ];
}

class ListI08 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'harray',
      'type' => TType::LST,
      'etype' => TType::I08,
      'elem' => dict[ 'type' => TType::I08 ],
    ],
  ];
}

class VectorI64 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'collection',
      'type' => TType::LST,
      'etype' => TType::I64,
      'elem' => dict[ 'type' => TType::I64 ],
    ],
  ];
}

class VectorI08 extends ThriftData {
  const SPEC = dict[
    1 => dict[
      'var' => 'data',
      'format' => 'collection',
      'type' => TType::LST,
      'etype' => TType::I08,
      'elem' => dict[ 'type' => TType::I08 ],
    ],
  ];
}

function test($name, $classname, $data) :mixed{
  $obj = new $classname();
  $obj->data = $data;

  echo "---- $name ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, '', 2, $obj, 20);
  var_dump($p->readCompactUsingAllMethods($classname));
}

function main(): mixed {
  test("empty ListI64", ListI64::class, vec[]);
  test("fast decoded ListI64", ListI64::class, vec[0x7FFFFFFFFFFFFF00, 0x7FFFFFFFFFFFFF01]);
  test("fast decoded ListI08", ListI08::class, vec[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);

  test("empty VectorI64", VectorI64::class, new Vector(null));
  test("fast decoded VectorI64", VectorI64::class, Vector{0x7FFFFFFFFFFFFF00, 0x7FFFFFFFFFFFFF01});
  test("fast decoded VectorI08", VectorI08::class, Vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
}

<<__EntryPoint>>
function main_collections() :mixed{
  require 'common.inc';
  main();
}
