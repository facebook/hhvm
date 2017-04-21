<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_varray($v) {
  echo "============== test_varray =========================\n";
  var_dump(varray($v));
}
function test_darray($v) {
  echo "============== test_darray =========================\n";
  var_dump(darray($v));
}

function test_indirect($c,  $v) {
  echo "============== test_indirect ($c) ==================\n";
  var_dump($c($v));
}

function test() {
  $values = vec[
    null,
    false,
    true,
    'abc',
    123,
    '123',
    1.234,
    [],
    [1, 2, 3, 4],
    vec[],
    vec[1, 2, 3, 4],
    dict[],
    dict[1 => 'a', 2 => 'b'],
    keyset[],
    keyset[100, 'abc', 200],
    xml_parser_create(),
    new stdclass(),
    dict[100 => 'abc', '100' => 'def'],
    keyset[100, '100']
  ];
  foreach ($values as $v) {
    test_varray($v);
  }
  foreach ($values as $v) {
    test_darray($v);
  }
  foreach ($values as $v) {
    test_indirect('HH\\varray', $v);
  }
  foreach ($values as $v) {
    test_indirect('HH\\darray', $v);
  }
}
test();
