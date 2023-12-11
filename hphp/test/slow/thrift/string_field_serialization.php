<?hh

class TestStruct {
  const SPEC = dict[
    1 => dict[
      'var' => 'var',
      'type' => TType::STRING,
    ],
  ];
  public function __construct(
    public $var = null,
  )[] {}
  public static function withDefaultValues()[]: this {
    return new static();
  }
  public function clearTerseFields()[write_props]: void {}
}

function _try($fn) :mixed{
  try {
    $fn();
  } catch (Exception $e) {
    echo "<".get_class($e).': '.$e->getMessage().">\n";
  }
}

function test_binary($var) :mixed{
  $p = new DummyProtocol();
  $v1 = new TestStruct($var);
  thrift_protocol_write_binary($p, 'foo', 1, $v1, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));
}

function test_compact($var) :mixed{
  $p = new DummyProtocol();
  $v1 = new TestStruct($var);
  thrift_protocol_write_compact2($p, 'foo', 2, $v1, 20);
  var_dump(thrift_protocol_read_compact($p, 'TestStruct'));
}

<<__EntryPoint>>
function test() :mixed{
  require 'common.inc';
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
