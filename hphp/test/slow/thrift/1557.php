<?php

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
}
class TestStruct {
  static $_TSPEC;
  public $aBool = null;
  public $anInt = null;
  public $aDouble = null;
  public $anInt64 = null;
  public $anByte = null;
  public $anI16 = null;
  public $aFloat = null;
  public $bFloat = null;
  public function __construct($vals=null) {
    if (!isset(self::$_TSPEC)) {
      self::$_TSPEC = array(        -1 => array(          'var' => 'aBool',          'type' => TType::BOOL,                   ),        1 => array(          'var' => 'anInt',          'type' => TType::I32,                   ),        2 => array(          'var' => 'aDouble',          'type' => TType::DOUBLE,                   ),        3 => array(          'var' => 'anInt64',          'type' => TType::I64,                   ),        4 => array(          'var' => 'anByte',          'type' => TType::BYTE,                  ),        5 => array(          'var' => 'anI16',          'type' => TType::I16,                  ),        6 => array(          'var' => 'aFloat',          'type' => TType::FLOAT,                   ),        7 => array(          'var' => 'bFloat',          'type' => TType::FLOAT,                   ),                    );
    }
  }
}
function test() {
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->aBool = false;
  $v1->anInt = -1234;
  $v1->aDouble = -1.2345;
  $v1->anInt64 = -1;
  $v1->anByte = -12;
  $v1->anI16 = -123;
  $v1->aFloat = 1.25;
  $v1->bFloat = 3.14159265358979323846264;
  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));
}
test();
