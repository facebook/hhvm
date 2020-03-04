<?hh

class TProtocolException extends Exception {}

class TType {
  const STRING = 11;
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
  function flush() {}
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
  const SPEC = array(
    1 => array(
      'var' => 'var',
      'type' => TType::STRING,
    ),
  );
  public function __construct(
    public $var = null,
  ) {}
}

function _try($fn) {
  try {
    $fn();
  } catch (Exception $e) {
    echo "<".get_class($e).': '.$e->getMessage().">\n";
  }
}

function test_binary($var) {
  $p = new DummyProtocol();
  $v1 = new TestStruct($var);
  thrift_protocol_write_binary($p, 'foo', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));
}

function test_compact($var) {
  $p = new DummyProtocol();
  $v1 = new TestStruct($var);
  thrift_protocol_write_compact($p, 'foo', 1, $v1, 20);
  $p->getTransport()->buff[1] = pack('C', 0x42);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));
}

<<__EntryPoint>>
function test() {
  echo "--- binary ---\n";
  _try(() ==> test_binary('asdf'));
  _try(() ==> test_binary(123));
  _try(() ==> test_binary(1.23));
  _try(() ==> test_binary(new stdClass()));

  echo "--- compact ---\n";
  _try(() ==> test_compact('asdf'));
  _try(() ==> test_compact(123));
  _try(() ==> test_compact(1.23));
  _try(() ==> test_compact(new stdClass()));
}
