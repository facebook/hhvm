<?hh

class InnerStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'value',
      'type' => \TType::I16,
    ],
  ];
  public $value = null;
  public function __construct($value = null)[] { $this->value = $value; }
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class OuterStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'value',
      'type' => \TType::STRING,
    ],
    2 => dict[
      'var' => 'nested',
      'type' => \TType::STRUCT,
      'class' => 'InnerStruct',
    ],
    3 => dict[
      'var' => 'collection',
      'type' => \TType::MAP,
      'ktype' => \TType::I16,
      'vtype' => \TType::STRUCT,
      'key' => dict[
        'type' => \TType::I16,
      ],
      'val' => dict[
        'type' => \TType::STRUCT,
        'class' => 'InnerStruct',
      ],
    ],
  ];
  public $value = null;
  public $nested = null;
  public $collection = null;
  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function getStruct() :mixed{
  $v = new OuterStruct();
  $v->value = "foo";
  $v->nested = new InnerStruct(42);
  $v->collection = Map {1 => new InnerStruct(2), 4 => new InnerStruct(8)};
  return $v;
}

function testBinary() :mixed{
  $p = new DummyProtocol();
  $v = getStruct();
  var_dump($v);
  thrift_protocol_write_binary($p, 'foomethod', 1, $v, 20, true);
  var_dump(md5($p->getTransport()->buff));
  var_dump(thrift_protocol_read_binary($p, 'OuterStruct', true));
}

function testCompact() :mixed{
  $p = new DummyProtocol();
  $v = getStruct();
  var_dump($v);
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v, 20);
  var_dump(md5($p->getTransport()->buff));
  var_dump($p->readCompactUsingAllMethods('OuterStruct','foomethod'));
}

<<__EntryPoint>>
function main_forward_compatibility() :mixed{
  require 'common.inc';
  echo "--- binary ---\n";
  testBinary();
  echo "--- compact ---\n";
  testCompact();
}
