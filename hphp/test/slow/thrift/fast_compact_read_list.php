<?hh

class TestStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'aList',
      'format' => 'harray',
      'type' => TType::LST,
      'etype' => TType::I32,
      'elem' => darray[ 'type' => TType::I32 ],
    ],
  ];

  public $aList = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

function test($name, $list) :mixed{
  $s = new TestStruct();
  $s->aList = $list;

  echo "---- $name: compact ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'foo', 2, $s, 20);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));
}

function main(): mixed {
  test("empty list", vec[]);
  test("small list", vec[1, 2, 3]);

  $data = vec[];
  for ($idx = 0; $idx < 100; $idx++) {
    $data[] = 1000000 * $idx;
  }
  test("big list", $data);
}

<<__EntryPoint>>
function main_collections() :mixed{
  require 'common.inc';
  main();
}
