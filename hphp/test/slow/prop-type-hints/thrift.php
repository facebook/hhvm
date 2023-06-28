<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function serde($x) :mixed{
  $p = new DummyProtocol();
  thrift_protocol_write_binary($p, 'blah', 1, $x, 20, true);
  echo "===================== binary deserializing ==================\n";
  thrift_protocol_read_binary($p, get_class($x), true);
  echo "=============================================================\n";

  if (!($x is TestStruct3)) {
    $p = new DummyProtocol();
    thrift_protocol_write_compact2($p, 'blah', 1, $x, 20, true);
    $p->getTransport()->buff[1] = pack('C', 0x42);
    echo "===================== compact deserializing ==================\n";
    thrift_protocol_read_compact($p, get_class($x));
    echo "==============================================================\n";
  }
}

<<__EntryPoint>>
function test() :mixed{
  require 'thrift-defs.inc';

  serde(@new TestStruct1());
  serde(@new TestStruct2());
  serde(@new TestStruct4());
  serde(@new TestStruct5());
  serde(@new TestStruct6());
  serde(@new TestStruct7());
  serde(@new TestStruct3());
}
