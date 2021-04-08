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

class OldStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'field1',
      'type' => TType::LST,
      'etype' => TType::LST,
      'elem' => darray[
        'type' => TType::LST,
        'etype' => TType::STRING,
        'elem' => darray[
          'type' => TType::STRING,
        ],
      ],
    ],
    2 => darray[
      'var' => 'field2',
      'type' => TType::LST,
      'etype' => TType::STRUCT,
      'elem' => darray[
        'type' => TType::STRUCT,
        'class' => 'InnerStruct',
      ],
    ],
  ];
  public $field1 = null;
  public $field2 = null;
  public function __construct() {}
}

class NewStruct {
  const SPEC = darray[
    1 => darray[
      'var' => 'field1',
      'type' => TType::LST,
      'etype' => TType::I16,
      'elem' => darray[
        'type' => TType::I16,
      ],
    ],
    2 => darray[
      'var' => 'field2',
      'type' => TType::LST,
      'etype' => TType::I16,
      'elem' => darray[
        'type' => TType::I16,
      ],
    ],
  ];
  public $field1 = null;
  public $field2 = null;
  public function __construct() {}
}

function testBinary($val) {
  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'OldStruct', 1, $val, 20, true);
  try {
    var_dump(thrift_protocol_read_binary($p, 'NewStruct', true));
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

function testCompact($val) {
  $p = new DummyProtocol();
  thrift_protocol_write_compact($p, 'OldStruct', 2, $val, 20);
  try {
    var_dump(thrift_protocol_read_compact($p, 'NewStruct'));
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

function main() {
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
function main_forward_compatibility() {
  require 'common.inc';
  main();
}
