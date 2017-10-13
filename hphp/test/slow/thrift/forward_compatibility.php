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

  function onewayFlush() {
  }

  function putBack($s) {
    $this->buff = ($s . $this->buff);
  }
}
class OldStruct {
  static $_TSPEC;
  public $features = null;
  public function __construct($vals=null) {
    if (!isset(self::$_TSPEC)) {
      self::$_TSPEC = array(
        1 => array(
          'var' => 'features',
          'type' => TType::MAP,
          'ktype' => TType::I16,
          'vtype' => TType::DOUBLE,
          'key' => array(
            'type' => TType::I16,
              ),
                'val' => array(
                      'type' => TType::DOUBLE,
              ),
          ),
      );
    }
  }
}

class NewStruct {
  static $_TSPEC;
  public $features = null;
  public function __construct($vals=null) {
    if (!isset(self::$_TSPEC)) {
      self::$_TSPEC = array(
        1 => array(
          'var' => 'features',
          'type' => TType::MAP,
          'ktype' => TType::I32,
          'vtype' => TType::FLOAT,
          'key' => array(
            'type' => TType::I32,
              ),
                'val' => array(
                      'type' => TType::FLOAT,
                        ),
              ),
      );
    }
  }
}

class TProtocolException extends Exception {}

function testBinary() {
  $p1 = new DummyProtocol();
  $v1 = new OldStruct();
  $v1->features = array(1 => 314, 2 => 271.8);
  thrift_protocol_write_binary($p1, 'foomethod', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p1, 'NewStruct', true));

  $p2 = new DummyProtocol();
  $v2 = new NewStruct();
  $v2->features = array(1 => 314, 2 => 271.8);
  thrift_protocol_write_binary($p2, 'foomethod', 1, $v2, 20, true);
  var_dump(thrift_protocol_read_binary($p2, 'OldStruct', true));
}

function testCompact() {
  $p1 = new DummyProtocol();
  $v1 = new OldStruct();
  $v1->features = array(1 => 314, 2 => 271.8);
  thrift_protocol_write_compact($p1, 'foomethod', 1, $v1, 20);
  $p1->getTransport()->buff[1] = pack('C', 0x42);
  var_dump(thrift_protocol_read_compact($p1, 'NewStruct'));

  $p2 = new DummyProtocol();
  $v2 = new NewStruct();
  $v2->features = array(1 => 314, 2 => 271.8);
  thrift_protocol_write_compact($p2, 'foomethod', 1, $v2, 20);
  $p2->getTransport()->buff[1] = pack('C', 0x42);
  var_dump(thrift_protocol_read_compact($p2, 'OldStruct'));
}

function main() {
  testBinary();
  testCompact();
}

main();
