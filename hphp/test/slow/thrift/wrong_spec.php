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

class OldStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'field1',
      'type' => TType::LST,
      'etype' => TType::LST,
      'elem' => dict[
        'type' => TType::LST,
        'etype' => TType::STRING,
        'elem' => dict[
          'type' => TType::STRING,
        ],
      ],
    ],
    2 => dict[
      'var' => 'field2',
      'type' => TType::LST,
      'etype' => TType::STRUCT,
      'elem' => dict[
        'type' => TType::STRUCT,
        'class' => 'InnerStruct',
      ],
    ],
  ];
  public $field1 = null;
  public $field2 = null;
  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class NewStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'field1',
      'type' => TType::LST,
      'etype' => TType::I16,
      'elem' => dict[
        'type' => TType::I16,
      ],
    ],
    2 => dict[
      'var' => 'field2',
      'type' => TType::LST,
      'etype' => TType::I16,
      'elem' => dict[
        'type' => TType::I16,
      ],
    ],
  ];
  public $field1 = null;
  public $field2 = null;
  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function testBinary($val) :mixed{
  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'OldStruct', 1, $val, 20, true);
  try {
    var_dump(thrift_protocol_read_binary($p, 'NewStruct', true));
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

function testCompact($val) :mixed{
  $p = new DummyProtocol();
  thrift_protocol_write_compact2($p, 'OldStruct', 2, $val, 20);
  try {
    var_dump(thrift_protocol_read_compact($p, 'NewStruct'));
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

function main() :mixed{
  $v = new OldStruct();
  $v->field1 = vec[vec[1, 2, 3]];
  testBinary($v);
  testCompact($v);

  $v = new OldStruct();
  $v->field2 = vec[new InnerStruct(123)];
  testBinary($v);
  testCompact($v);
}


<<__EntryPoint>>
function main_forward_compatibility() :mixed{
  require 'common.inc';
  main();
}
