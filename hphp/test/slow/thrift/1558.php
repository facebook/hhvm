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
  public $aString = null;
  public $aDouble = null;
  public $anInt64 = null;
  public $aList = null;
  public $aMap = null;
  public $aSet = null;
  public $anByte = null;
  public $anI16 = null;
  public $aFloat = null;

      public function __construct($vals=null) {
    if (!isset(self::$_TSPEC)) {
      self::$_TSPEC = array(        -1 => array(          'var' => 'aBool',          'type' => TType::BOOL,                   ),        1 => array(          'var' => 'anInt',          'type' => TType::I32,                   ),        2 => array(          'var' => 'aString',          'type' => TType::STRING,                   ),        3 => array(          'var' => 'aDouble',          'type' => TType::DOUBLE,                   ),        4 => array(          'var' => 'anInt64',          'type' => TType::I64,                   ),        5 => array(          'var' => 'aList',          'type' => TType::LST,          'etype' => TType::DOUBLE,          'elem' => array(            'type' => TType::DOUBLE,                          ),                   ),        6 => array(          'var' => 'aMap',          'type' => TType::MAP,          'ktype' => TType::I32,          'vtype' => TType::DOUBLE,          'key' => array(            'type' => TType::I32,                         ),          'val' => array(            'type' => TType::DOUBLE,                         ),          ),       7 => array(          'var' => 'aSet',          'type' => TType::SET,          'etype' => TType::I32,          'elem' => array(            'type' => TType::I32,                          ),                  ),       8 => array(          'var' => 'anByte',          'type' => TType::BYTE,                  ),       9 => array(          'var' => 'anI16',          'type' => TType::I16,                  ),       10 => array(          'var' => 'aFloat',          'type' => TType::FLOAT,                  ),                            );
    }
  }
}
function test() {
  $p = new DummyProtocol();
  $v1 = new TestStruct();
  $v1->aBool = true;
  $v1->anInt = 1234;
  $v1->aString = 'abcdef';
  $v1->aDouble = 1.2345;
  $v1->anInt64 = 8589934592;
  $v1->aList = array(13.3, 23.4, 3576.2);
  $v1->aMap = array(10=>1.2, 43=>5.33);
  $v1->aSet = array(10=>true, 11=>true);
  $v1->anByte = 123;
  $v1->anI16 = 1234;
  $v1->aFloat = 1.25;
  thrift_protocol_write_compact($p, 'foomethod', 1, $v1, 20);
  var_dump(md5($p->getTransport()->buff));
}
test();
