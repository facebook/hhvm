<?hh

class TestStruct {
  <<ThriftDeprecatedSkipSerializerChecks>>
  public $anI32 = null;
  <<ThriftDeprecatedSkipSerializerChecks>>
  public $anI16 = null;
  <<ThriftDeprecatedSkipSerializerChecks>>
  public $map = null;

  const SPEC = dict[
    1 => dict[
      'var' => 'anI32',
      'type' => TType::I32,
    ],
    2 => dict[
      'var' => 'anI16',
      'type' => TType::I16,
    ],
    3 => dict[
      'var' => 'map',
      'type' => TType::MAP,
      'ktype' => TType::I16,
      'vtype' => TType::I16,
      'key' => dict[
        'type' => TType::I16,
      ],
      'val' => dict[
        'type' => TType::I16,
      ],
    ],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

<<__EntryPoint>>
function test() :mixed{
  require 'common.inc';
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->anI32 = 1 << 31;
  $v1->anI16 = 1 << 15;
  $v1->map = dict[(1 << 15) => 0];
  thrift_protocol_write_compact2($p, 'foomethod', 1, $v1, 20, true);
}
