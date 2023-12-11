<?hh

class TestStruct {
  const SPEC = dict[
    5 => dict[
      'var' => 'aList',
      'format' => 'collection',
      'type' => TType::LST,
      'etype' => TType::DOUBLE,
      'elem' => dict[ 'type' => TType::DOUBLE ],
    ],
    6 => dict[
      'var' => 'aMap',
      'format' => 'collection',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::DOUBLE,
      'key' => dict[ 'type' => TType::I32 ],
      'val' => dict[ 'type' => TType::DOUBLE ],
    ],
    7 => dict[
      'var' => 'aSet',
      'format' => 'collection',
      'type' => TType::SET,
      'etype' => TType::I32,
      'elem' => dict[ 'type' => TType::I32 ],
    ],
  ];

  public $aList = null;
  public $aMap = null;
  public $aSet = null;
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function test($name, $list, $map, $set) :mixed{
  $s = new TestStruct();
  $s->aList = $list;
  $s->aMap = $map;
  $s->aSet = $set;

  echo "---- $name: compact ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'foo', 2, $s, 20);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));

  echo "---- $name: binary ----\n";
  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'foo', 1, $s, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));
}

function main() :mixed{
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
    vec[13.3, 23.4, 3576.2],
    dict[10 => 1.2, 43 => 5.33],
    dict[10 => 'doesnt', 11 => 'matter']
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
function main_collections() :mixed{
  require 'common.inc';
  main();
}
