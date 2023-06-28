<?hh

interface IThriftStruct {
  public function getName():mixed;
}

class NodeObject {
  const SPEC = darray[
    1 => darray[
      'var' => 'fbid',
      'type' => TType::I64,
    ],
    2 => darray[
      'var' => 'type',
      'type' => TType::I64,
    ],
    3 => darray[
      'var' => 'owner',
      'type' => TType::I64,
    ],
  ];
  public static $_TFIELDMAP = darray[
    'fbid' => 1,
    'type' => 2,
    'owner' => 3,
  ];
  const STRUCTURAL_ID = 5073716637340378305;
  public $fbid = null;
  public $type = null;
  public $owner = null;

  public function __construct($vals=null)[] {
    if (is_array($vals)) {
      if (isset($vals['fbid'])) {
        $this->fbid = $vals['fbid'];
      }
      if (isset($vals['type'])) {
        $this->type = $vals['type'];
      }
      if (isset($vals['owner'])) {
        $this->owner = $vals['owner'];
      }
    } else if ($vals) {
      throw new Exception(
        'NodeObject constructor must be passed array or null'
      );
    }
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}

  public function getName() :mixed{
    return 'NodeObject';
  }

  public static function __set_state($vals) :mixed{
    return new NodeObject($vals);
  }
}

class EdgeObject {
  const SPEC = darray[
    1 => darray[
      'var' => 'source',
      'type' => TType::STRUCT,
      'class' => 'NodeObject',
    ],
    2 => darray[
      'var' => 'target',
      'type' => TType::STRUCT,
      'class' => 'NodeObject',
    ],
    3 => darray[
      'var' => 'type',
      'type' => TType::I64,
    ],
    4 => darray[
      'var' => 'timeCreated',
      'type' => TType::I64,
    ],
    5 => darray[
      'var' => 'creator',
      'type' => TType::STRUCT,
      'class' => 'NodeObject',
    ],
    6 => darray[
      'var' => 'actionSource',
      'type' => TType::I32,
      'enum' => 'ActionSource',
    ],
    7 => darray[
      'var' => 'appId',
      'type' => TType::I64,
    ],
  ];
  public static $_TFIELDMAP = darray[
    'source' => 1,
    'target' => 2,
    'type' => 3,
    'timeCreated' => 4,
    'creator' => 5,
    'actionSource' => 6,
    'appId' => 7,
  ];
  const STRUCTURAL_ID = 8017252320845196830;
  public $source = null;
  public $target = null;
  public $type = null;
  public $timeCreated = null;
  public $creator = null;
  public $actionSource = null;
  public $appId = null;

  public function __construct($vals=null)[] {
    if (is_dict($vals)) {
      if (isset($vals['source'])) {
        $this->source = $vals['source'];
      }
      if (isset($vals['target'])) {
        $this->target = $vals['target'];
      }
      if (isset($vals['type'])) {
        $this->type = $vals['type'];
      }
      if (isset($vals['timeCreated'])) {
        $this->timeCreated = $vals['timeCreated'];
      }
      if (isset($vals['creator'])) {
        $this->creator = $vals['creator'];
      }
      if (isset($vals['actionSource'])) {
        $this->actionSource = $vals['actionSource'];
      }
      if (isset($vals['appId'])) {
        $this->appId = $vals['appId'];
      }
    } else if ($vals) {
      throw new Exception(
                'EdgeObject constructor must be passed array or null'
      );
    }
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}

  public function getName() :mixed{
    return 'EdgeObject';
  }

  public static function __set_state($vals) :mixed{
    return new EdgeObject($vals);
  }
}

class EdgeObjectWithBadSpec1 extends EdgeObject {
  const SPEC = darray[
    7 => darray[
      'var' => 'appId',
      'type' => TType::STRING,
    ],
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class EdgeObjectWithBadSpec2 extends EdgeObject {
  const SPEC = varray[];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class EdgeObjectWithBadSpec3 extends EdgeObject {
  const SPEC = 42;

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

class EdgeObjectWithBadSpec4 extends EdgeObject {
  const SPEC = darray[
    'foo' => 'bar',
  ];

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function clearTerseFields()[write_props]: void {}
}

function testBadSpec($bad) :mixed{
  $p = new DummyProtocol();
  $v1 = new EdgeObject();
  $v1->appId = 1234;
  $v1->timeCreated = 5678;
  $v1->actionSource = 42;
  $v1->type = 87;
  thrift_protocol_write_compact2($p, 'foomethod', 2, $v1, 20);
  var_dump(md5($p->getTransport()->buff));

  $p->getTransport()->pos = 0;
  var_dump(thrift_protocol_read_compact($p, 'EdgeObject'));

  $p->getTransport()->pos = 0;
  try {
    var_dump(thrift_protocol_read_compact($p, $bad));
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main_bad_spec() :mixed{
  require 'common.inc';
  testBadSpec('EdgeObjectWithBadSpec1');
  testBadSpec('EdgeObjectWithBadSpec2');
  testBadSpec('EdgeObjectWithBadSpec3');
  testBadSpec('EdgeObjectWithBadSpec4');
  testBadSpec('EdgeObjectWithBadSpec4');
}
