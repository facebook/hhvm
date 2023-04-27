<?hh

class OldStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'features',
      'type' => TType::MAP,
      'ktype' => TType::I16,
      'vtype' => TType::DOUBLE,
      'key' => darray[
        'type' => TType::I16,
      ],
      'val' => darray[
        'type' => TType::DOUBLE,
      ],
    ],
  ];
  public $features = null;
  public function __construct($vals=null)[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class NewStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'features',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::FLOAT,
      'key' => darray[
        'type' => TType::I32,
      ],
      'val' => darray[
        'type' => TType::FLOAT,
      ],
    ],
  ];
  public $features = null;
  public function __construct($vals=null)[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function testBinary() {
  $p1 = new DummyProtocol();
  $v1 = new OldStruct();
  $v1->features = darray[1 => 314, 2 => 271.8];
  thrift_protocol_write_binary($p1, 'foomethod', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p1, 'NewStruct', true));

  $p2 = new DummyProtocol();
  $v2 = new NewStruct();
  $v2->features = darray[1 => 314, 2 => 271.8];
  thrift_protocol_write_binary($p2, 'foomethod', 1, $v2, 20, true);
  var_dump(thrift_protocol_read_binary($p2, 'OldStruct', true));
}

function testCompact() {
  $p1 = new DummyProtocol();
  $v1 = new OldStruct();
  $v1->features = darray[1 => 314, 2 => 271.8];
  thrift_protocol_write_compact2($p1, 'foomethod', 2, $v1, 20);
  var_dump(thrift_protocol_read_compact($p1, 'NewStruct'));

  $p2 = new DummyProtocol();
  $v2 = new NewStruct();
  $v2->features = darray[1 => 314, 2 => 271.8];
  thrift_protocol_write_compact2($p2, 'foomethod', 2, $v2, 20);
  var_dump(thrift_protocol_read_compact($p2, 'OldStruct'));
}

function main() {
  testBinary();
  testCompact();
}


<<__EntryPoint>>
function main_forward_compatibility() {
  require 'common.inc';
  main();
}
