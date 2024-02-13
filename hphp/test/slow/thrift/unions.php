<?hh

enum ComplexUnionEnum: int {
  _EMPTY_ = 0;
  intValue = 1;
  stringValue = 2;
}
class ComplexUnion {
  const AnyArray SPEC = dict[
    1 => dict[
      'var' => 'intValue',
      'union' => true,
      'type' => TType::I64,
      ],
    2 => dict[
      'var' => 'stringValue',
      'union' => true,
      'type' => TType::STRING,
      ],
    ];
  public static Map<string, int> $_TFIELDMAP = Map {
    'intValue' => 1,
    'stringValue' => 2,
  };
  public ?int $intValue;
  public ?string $stringValue;
  protected ComplexUnionEnum $_type = ComplexUnionEnum::_EMPTY_;

  public function __construct(?int $intValue = null, ?string $stringValue = null)[] {
    $this->_type = ComplexUnionEnum::_EMPTY_;
    if ($intValue !== null) {
      $this->intValue = $intValue;
      $this->_type = ComplexUnionEnum::intValue;
    }
    if ($stringValue !== null) {
      $this->stringValue = $stringValue;
      $this->_type = ComplexUnionEnum::stringValue;
    }
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}

  public function getName(): string {
    return 'ComplexUnion';
  }

  public function getType(): ComplexUnionEnum {
    return $this->_type;
  }

  public function set_intValue(int $intValue): void {
    $this->_type = ComplexUnionEnum::intValue;
    $this->intValue = $intValue;
  }

  public function get_intValue(): int {
    invariant($this->_type === ComplexUnionEnum::intValue,
      'get_intValue called on an instance of ComplexUnion whose current type is' . $this->_type);
    return nullthrows($this->intValue);
  }

  public function set_stringValue(string $stringValue): void {
    $this->_type = ComplexUnionEnum::stringValue;
    $this->stringValue = $stringValue;
  }

  public function get_stringValue(): string {
    invariant($this->_type === ComplexUnionEnum::stringValue,
      'get_stringValue called on an instance of ComplexUnion whose current type is' . $this->_type);
    return nullthrows($this->stringValue);
  }

  public function read(TProtocol $input): int {
    $xfer = 0;
    $fname = '';
    $ftype = 0;
    $fid = 0;
    $this->_type = ComplexUnionEnum::_EMPTY_;
    $xfer += $input->readStructBegin($fname);
    while (true)
    {
      $xfer += $input->readFieldBegin($fname, $ftype, $fid);
      if ($ftype == TType::STOP) {
        break;
      }
      if (!$fid && $fname !== null) {
        $fid = (int) self::$_TFIELDMAP->get($fname);
        if ($fid !== 0) {
          $ftype = self::SPEC[$fid]['type'];
        }
      }
      switch ($fid)
      {
        case 1:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->intValue);
            $this->_type = ComplexUnionEnum::intValue;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 2:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->stringValue);
            $this->_type = ComplexUnionEnum::stringValue;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        default:
          $xfer += $input->skip($ftype);
          break;
      }
      $xfer += $input->readFieldEnd();
    }
    $xfer += $input->readStructEnd();
    return $xfer;
  }

  public function write(TProtocol $output): int {
    $xfer = 0;
    $xfer += $output->writeStructBegin('ComplexUnion');
    if ($this->intValue !== null) {
      $_val0 = $this->intValue;
      $xfer += $output->writeFieldBegin('intValue', TType::I64, 1);
      $xfer += $output->writeI64($_val0);
      $xfer += $output->writeFieldEnd();
    }
    if ($this->stringValue !== null) {
      $_val1 = $this->stringValue;
      $xfer += $output->writeFieldBegin('stringValue', TType::STRING, 2);
      $xfer += $output->writeString($_val1);
      $xfer += $output->writeFieldEnd();
    }
    $xfer += $output->writeFieldStop();
    $xfer += $output->writeStructEnd();
    return $xfer;
  }
}

<<__EntryPoint>> function test(): void {
  require 'common.inc';

  $p = new DummyProtocol();
  $v1 = new ComplexUnion();
  $v1->set_stringValue('What is the answer?');
  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'ComplexUnion', true));

  $p = new DummyProtocol();
  $v1 = new ComplexUnion();
  $v1->set_intValue(42);
  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'ComplexUnion', true));

  $p = new DummyProtocol();
  $v1 = new ComplexUnion();
  $v1->set_stringValue('What is the answer?');
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v1, 20);
  var_dump($p->readCompactUsingAllMethods('ComplexUnion','foomethod'));

  $p = new DummyProtocol();
  $v1 = new ComplexUnion();
  $v1->set_intValue(42);
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v1, 20);
  var_dump($p->readCompactUsingAllMethods('ComplexUnion','foomethod'));
}
