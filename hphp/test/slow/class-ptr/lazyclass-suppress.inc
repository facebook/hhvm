<?hh

class TProtocolException extends Exception {}

class TType {
  const STOP   = 0;
  const VOID   = 1;
  const BOOL   = 2;
  const BYTE   = 3;
  const I08    = 3;
  const DOUBLE = 4;
  const I16    = 6;
  const I32    = 8;
  const I64    = 10;
  const STRING = 11;
  const UTF7   = 11;
  const STRUCT = 12;
  const MAP    = 13;
  const SET    = 14;
  const LIST   = 15;
  const UTF8   = 16;
  const UTF16  = 17;
  const FLOAT  = 19;
}

class ThriftStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'str',
      'type' => \TType::STRING,
    ],
  ];

  public string $str;

  public function __construct(string $str)[] {
    $this->str = $str;
  }

  public static function withDefaultValues()[]: this {
    return new static("");
  }

  public function clearTerseFields()[write_props]: void {}
}

class NestedStruct {
  public string $str;

  public function __construct(string $str) {
    $this->str = $str;
  }

  public function withDefaultValues()[]: this {
    return new static("");
  }

  public function clearTerseFields()[write_props]: void {}
};

class NestedStructAdapter {
  const type THackType = NestedStruct;

  public static function toThrift(NestedStruct $hack_value): ThriftStruct {
    return new ThriftStruct(HH\Lib\Str\reverse($hack_value->str));
  }

  public static function fromThrift(ThriftStruct $thrift_struct): NestedStruct {
    return new NestedStruct(HH\Lib\Str\reverse($thrift_struct->str));
  }
};

class Foo {
  const SPEC = dict[
    1 => dict[
      'var' => 'nested',
      'type' => \TType::STRUCT,
      'class' => ThriftStruct::class,
      'adapter' => NestedStructAdapter::class
    ],
  ];

  public ?NestedStructAdapter::THackType $nested = null;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
};

class DummyProtocol {
  public $t;
  function __construct() { $this->t = new DummyTransport(); }
  function getTransport() { return $this->t; }
}

class DummyTransport {
  public $buff = '';
  public $pos = 0;

  function write($buff) {
    $this->buff .= $buff;
  }

  function flush() {
  }

  function read($n) {
    $r = substr($this->buff, $this->pos, $n);
    $this->pos += strlen($r);
    return $r;
  }

  function putback($s): void {
    $n = \strlen($s);
    echo "@@@ putback $n at pos ", $this->pos, "\n";
    invariant($n >= $this->pos, __METHOD__);
    invariant($s === \substr($this->buff, $this->pos - $n, $n), __METHOD__);
    $this->pos -= $n;
  }
}

type FooTransparentAlias = Foo;
newtype FooOpaqueAlias = Foo;

class Baz {
  const type T = Foo;
};

class Bang {
  const type U = Baz::T;
};
