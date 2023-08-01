<?hh

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

class TProtocolException extends Exception {
  const UNKNOWN = 0;
  const INVALID_DATA = 1;
  const NEGATIVE_SIZE = 2;
  const SIZE_LIMIT = 3;
  const BAD_VERSION = 4;

  function __construct($message=null, $code=0)[] {
    parent::__construct($message, $code);
  }
}

class DummyProtocol {
  public $t;
  function __construct()[] {
    $this->t = new DummyTransport();
  }
  function getTransport() {
    return $this->t;
  }
}

class DummyTransport {
  public $buff = '';
  public $pos = 0;
  function flush() { }
  function onewayFlush() {}
  function write($buff) {
    $this->buff .= $buff;
  }
  function read($n) {
    $rem = strlen($this->buff) - $this->pos;
    if ($rem === 0) {
        throw new Exception("Out of input");
    } else if ($rem < $n) {
        $n = $rem;
    }
    $r = \substr($this->buff, $this->pos, $n);
    $this->pos += $n;
    return $r;
  }
  function putBack($data) {
    $this->buff = ($data . $this->buff);
  }
}
