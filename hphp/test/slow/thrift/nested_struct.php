<?hh

class InnerStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::I16,
    ],
  ];
  public $value = null;
  public function __construct($value = null) { $this->value = $value; }
}

class OuterStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'value',
      'type' => \TType::STRING,
    ],
    2 => darray[
      'var' => 'nested',
      'type' => \TType::STRUCT,
      'class' => 'InnerStruct',
    ],
    3 => darray[
      'var' => 'collection',
      'type' => \TType::MAP,
      'ktype' => \TType::I16,
      'vtype' => \TType::STRUCT,
      'key' => darray[
        'type' => \TType::I16,
      ],
      'val' => darray[
        'type' => \TType::STRUCT,
        'class' => 'InnerStruct',
      ],
    ],
  ];
  public $value = null;
  public $nested = null;
  public $collection = null;
  public function __construct() {}
}

function getStruct() {
  $v = new OuterStruct();
  $v->value = "foo";
  $v->nested = new InnerStruct(42);
  $v->collection = Map {1 => new InnerStruct(2), 4 => new InnerStruct(8)};
  return $v;
}

function testBinary() {
  $p = new DummyProtocol();
  $v = getStruct();
  var_dump($v);
  thrift_protocol_write_binary($p, 'foomethod', 1, $v, 20, true);
  var_dump(md5($p->getTransport()->buff));
  var_dump(thrift_protocol_read_binary($p, 'OuterStruct', true));
}

function testCompact() {
  $p = new DummyProtocol();
  $v = getStruct();
  var_dump($v);
  thrift_protocol_write_compact($p, 'foomethod', 2, $v, 20);
  var_dump(md5($p->getTransport()->buff));
  var_dump(thrift_protocol_read_compact($p, 'OuterStruct'));
}

<<__EntryPoint>>
function main_forward_compatibility() {
  require 'common.inc';
  echo "--- binary ---\n";
  testBinary();
  echo "--- compact ---\n";
  testCompact();
}
