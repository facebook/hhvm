<?hh

class ThriftStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'int_value',
      'type' => \TType::I32,
    ],
  ];
  public ?int $int_value = null;

  public function __construct(?int $int_value = null)[] {
    $this->int_value = $int_value;
  }
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

class HackStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'str_value',
      'type' => \TType::STRING,
    ],
  ];

  public ?string $str_value = null;

  public function __construct(?string $str_value = null)[] {
    $this->str_value = $str_value;
  }
}

class StringToIntPrimitiveAdapter {
  const type THackType = string;

  public static function toThrift(string $hack_value): int {
    return (int)$hack_value;
  }
  public static function fromThrift(int $thrift_value): string {
    return (string)$thrift_value;
  }
}

class StringToIntStructAdapter {
  const type THackType = HackStruct;

  public static function toThrift(HackStruct $hack_struct): ThriftStruct {
    return new ThriftStruct((int)$hack_struct->str_value);
  }
  public static function fromThrift(ThriftStruct $thrift_struct): HackStruct {
    return new HackStruct((string)$thrift_struct->int_value);
  }
}

class OuterStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'value',
      'type' => \TType::I32,
      'adapter' => StringToIntPrimitiveAdapter::class,
    ],
    2 => dict[
      'var' => 'nested',
      'type' => \TType::STRUCT,
      'class' => ThriftStruct::class,
      'adapter' => StringToIntStructAdapter::class,
    ],
    3 => dict[
      'var' => 'vec',
      'type' => \TType::LST,
      'etype' => \TType::STRUCT,
      'elem' => dict[
        'type' => \TType::STRUCT,
        'class' => ThriftStruct::class,
        'adapter' => StringToIntStructAdapter::class
      ],
      'format' => 'harray',
    ],
    4 => dict[
      'var' => 'unset',
      'type' => \TType::STRUCT,
      'class' => ThriftStruct::class,
      'adapter' => StringToIntStructAdapter::class,
    ],
  ];
  public ?StringToIntPrimitiveAdapter::THackType $value = null;
  public ?StringToIntStructAdapter::THackType $nested = null;
  public ?vec<StringToIntStructAdapter::THackType> $vec = null;
  public ?StringToIntStructAdapter::THackType $unset = null;
  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

// This class is identical to OuterStruct but with all the adapters removed.
// It's used to "peek" at the actual serialized data without adapters getting
// in the way.
class OuterStructNoAdapter {
  const SPEC = dict[
    1 => dict[
      'var' => 'value',
      'type' => \TType::I32,
    ],
    2 => dict[
      'var' => 'nested',
      'type' => \TType::STRUCT,
      'class' => ThriftStruct::class
    ],
    3 => dict[
      'var' => 'vec',
      'type' => \TType::LST,
      'etype' => \TType::STRUCT,
      'elem' => dict[
        'type' => \TType::STRUCT,
        'class' => ThriftStruct::class,
      ],
      'format' => 'harray',
    ],
    4 => dict[
      'var' => 'unset',
      'type' => \TType::STRUCT,
      'class' => ThriftStruct::class
    ],
  ];
  public ?int $value = null;
  public ?ThriftStruct $nested = null;
  public ?vec<ThriftStruct> $vec = null;
  public ?ThriftStruct $unset = null;
  public function __construct()[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function getStruct() :mixed{
  $v = new OuterStruct();
  $v->value = "42";
  $v->nested = new HackStruct("1337");
  $v->vec = vec[new HackStruct("2020"), new HackStruct("-1")];
  return $v;
}

function testBinary() :mixed{
  $p = new DummyProtocol();
  $v = getStruct();
  var_dump($v);
  thrift_protocol_write_binary($p, 'foomethod', 1, $v, 20, true);
  var_dump(md5($p->getTransport()->buff));
  var_dump(thrift_protocol_read_binary($p, 'OuterStruct', true));

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  var_dump(thrift_protocol_read_binary($p, 'OuterStructNoAdapter', true));
}

function testCompact() :mixed{
  $p = new DummyProtocol();
  $v = getStruct();
  var_dump($v);
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v, 20);
  var_dump(md5($p->getTransport()->buff));
  var_dump(thrift_protocol_read_compact($p, 'OuterStruct'));

  // Peek at what the serialized data actually looks like.
  $p->getTransport()->pos = 0;
  var_dump(thrift_protocol_read_compact($p, 'OuterStructNoAdapter'));
}

<<__EntryPoint>>
function main_forward_compatibility() :mixed{
  require 'common.inc';
  echo "--- binary ---\n";
  testBinary();
  echo "--- compact ---\n";
  testCompact();
}
