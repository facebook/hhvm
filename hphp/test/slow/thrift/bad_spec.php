<?php

class TProtocolException extends Exception {
  const UNKNOWN = 0;
  const INVALID_DATA = 1;
  const NEGATIVE_SIZE = 2;
  const SIZE_LIMIT = 3;
  const BAD_VERSION = 4;

  function __construct($message=null, $code=0) {
    parent::__construct($message, $code);
  }
}

interface IThriftStruct {
  public function getName();
}

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
  const LST    = 15;
  const UTF8   = 16;
  const UTF16  = 17;
  const FLOAT  = 19;
}

class DummyProtocol {
  public $t;
  function __construct() {
    $this->t = new DummyTransport();
  }
  function getTransport() {
    return $this->t;
  }
}

class DummyTransport {
  public $buff = '';
  public $pos = 0;
  function flush() {
 }
  function write($buff) {
    $this->buff .= $buff;
  }
   function read($n) {
    $r = substr($this->buff, $this->pos, $n);
    $this->pos += $n;
    return $r;
  }
   function putBack($data) {
    $this->buff = ($data . $this->buff);
  }
}

class NodeObject {
  static $_TSPEC = array(
    1 => array(
      'var' => 'fbid',
      'type' => TType::I64,
    ),
    2 => array(
      'var' => 'type',
      'type' => TType::I64,
    ),
    3 => array(
      'var' => 'owner',
      'type' => TType::I64,
    ),
  );
  public static $_TFIELDMAP = array(
    'fbid' => 1,
    'type' => 2,
    'owner' => 3,
  );
  const STRUCTURAL_ID = 5073716637340378305;
  public $fbid = null;
  public $type = null;
  public $owner = null;

  public function __construct($vals=null) {
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

  public function getName() {
    return 'NodeObject';
  }

  public static function __set_state($vals) {
    return new NodeObject($vals);
  }
}

class EdgeObject {
  static $_TSPEC = array(
    1 => array(
      'var' => 'source',
      'type' => TType::STRUCT,
      'class' => 'NodeObject',
    ),
    2 => array(
      'var' => 'target',
      'type' => TType::STRUCT,
      'class' => 'NodeObject',
    ),
    3 => array(
      'var' => 'type',
      'type' => TType::I64,
    ),
    4 => array(
      'var' => 'timeCreated',
      'type' => TType::I64,
    ),
    5 => array(
      'var' => 'creator',
      'type' => TType::STRUCT,
      'class' => 'NodeObject',
    ),
    6 => array(
      'var' => 'actionSource',
      'type' => TType::I32,
      'enum' => 'ActionSource',
    ),
    7 => array(
      'var' => 'appId',
      'type' => TType::I64,
    ),
  );
  public static $_TFIELDMAP = array(
    'source' => 1,
    'target' => 2,
    'type' => 3,
    'timeCreated' => 4,
    'creator' => 5,
    'actionSource' => 6,
    'appId' => 7,
  );
  const STRUCTURAL_ID = 8017252320845196830;
  public $source = null;
  public $target = null;
  public $type = null;
  public $timeCreated = null;
  public $creator = null;
  public $actionSource = null;
  public $appId = null;

  public function __construct($vals=null) {
    if (is_array($vals)) {
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

  public function getName() {
    return 'EdgeObject';
  }

  public static function __set_state($vals) {
    return new EdgeObject($vals);
  }

}

function testBadSpec($ok, $bad) {
  EdgeObject::$_TSPEC = $ok;

  $p = new DummyProtocol();
  $v1 = new EdgeObject();
  $v1->appId = 1234;
  $v1->timeCreated = 5678;
  $v1->actionSource = 42;
  $v1->type = 87;
  thrift_protocol_write_compact($p, 'foomethod', 1, $v1, 20);
  var_dump($p->getTransport()->buff);
  $p->getTransport()->buff[1] = pack('C', 0x42);

  $p->getTransport()->pos = 0;
  var_dump(thrift_protocol_read_compact($p, 'EdgeObject'));

  EdgeObject::$_TSPEC = $bad;

  $p->getTransport()->pos = 0;
  try {
    var_dump(thrift_protocol_read_compact($p, 'EdgeObject'));
  } catch (TProtocolException $e) {
    echo $e->getMessage() . "\n";
  }
}
$ok = EdgeObject::$_TSPEC;
testBadSpec($ok, array(7 => array('var' => 'appId', 'type' => TType::STRING)));
testBadSpec($ok, array());
testBadSpec($ok, 42);
testBadSpec($ok, array('foo' => 'bar'));
