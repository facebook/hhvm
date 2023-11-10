<?hh

final class TestStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'anInt1',
      'type' => TType::I32,
    ],
  ];

  public ?int $anInt1;

  public function __construct(?int $anInt1 = null)[] {
    $this->anInt1 = $anInt1;
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }
}

final class TestStructWithClearTerseFields {
  const SPEC = dict[
    1 => dict[
      'var' => 'anInt1',
      'type' => TType::I32,
    ],
    2 => dict[
      'var' => 'anInt2',
      'type' => TType::I32,
    ],
  ];

  public ?int $anInt1;
  public ?int $anInt2;

  public function __construct(?int $anInt1 = null, ?int $anInt2 = null)[] {
    $this->anInt1 = $anInt1 ?? 1;
    $this->anInt2 = $anInt2 ?? 2;
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {
    $this->anInt1 = null;
    $this->anInt2 = null;
  }
}

final class TestStructWithoutClearTerseFields {
  const SPEC = dict[
    1 => dict[
      'var' => 'anInt1',
      'type' => TType::I32,
    ],
    2 => dict[
      'var' => 'anInt2',
      'type' => TType::I32,
    ],
  ];

  public ?int $anInt1;
  public ?int $anInt2;

  public function __construct(?int $anInt1 = null, ?int $anInt2 = null)[] {
    $this->anInt1 = $anInt1 ?? 1;
    $this->anInt2 = $anInt2 ?? 2;
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }
}



<<__EntryPoint>>
async function main(): Awaitable<void>{
  require 'common.inc';

  $p = new DummyProtocol();
  $v = new TestStruct();
  $v->anInt1 = 3;
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v, 20);
  var_dump(thrift_protocol_read_compact($p, TestStructWithClearTerseFields::class));

  $p = new DummyProtocol();
  $v = new TestStruct();
  $v->anInt1 = 3;
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v, 20);
  var_dump(thrift_protocol_read_compact($p, TestStructWithoutClearTerseFields::class));
}
