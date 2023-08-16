<?hh

class TestStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'aMap',
      'format' => 'harray',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::I64,
      'key' => darray['type' => TType::I32],
      'val' => darray['type' => TType::I64],
    ],
  ];

  public $aMap = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

function test($name, $map) :mixed{
  $s = new TestStruct();
  $s->aMap = $map;

  echo "---- $name: compact ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'foo', 2, $s, 20);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));
}

function main(): mixed {
  test("empty map", Map{});
  test("small map", dict[1 => 111, 2 => 222]);

  $data = dict[];
  for ($key = 1; $key < 100; $key++) {
    $data[$key] = 1000000 * $key;
  }
  test("big map", $data);
}

<<__EntryPoint>>
function main_collections() :mixed{
  require 'common.inc';
  main();
}
