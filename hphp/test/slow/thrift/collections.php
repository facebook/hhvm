<?hh

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
function main_collections() :mixed{
  require 'common.inc';
  main();
}
