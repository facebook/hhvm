<?hh

<<__EntryPoint>>
function main(): void {
  require 'common.inc';

  // Ensure that if we ask for more data than is in the string we get "Out of
  // input" instead of "Out of memory". These numbers were chosen because
  // they're too big for VecInit - but we should never get to that point.
  foreach (vec["E38E38E2", "FFFFFFFF"] as $sz) {
    $p = new DummyProtocol();
    $p->t->buff = hex2bin("8001000100000009666f6f6d6574686f64000000140f00050a".$sz."000000000000000100");
    try {
      $r = thrift_protocol_read_binary($p, 'TestStruct', true);
    } catch(Exception $e) {
      if ($e->getMessage() != "Out of input") {
        throw $e;
      }
    }
  }

  echo "Success\n";
}

class TestStruct {
  const SPEC = dict[
    5 => dict[
      'var' => 'aList',
      'type' => TType::LST,
      'etype' => TType::I64,
      'elem' => dict[
        'type' => TType::I64,
      ],
      'is_terse' => true,
    ]
  ];
  public $aList = vec[];

  public function __construct($vals=null)[] { }

  public static function withDefaultValues()[]: this { return new static(); }

  public function clearTerseFields()[write_props]: void {
    $this->aList = vec[];
  }
}
