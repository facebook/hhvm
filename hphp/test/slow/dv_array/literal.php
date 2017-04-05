<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_varray() {
  var_dump(varray[]);
  var_dump(varray[1, 2, 3, 4]);
  var_dump(varray['a', 'b', 'c']);
  var_dump(varray[null, true, false, 'abc', 100, 1.23,
                  new stdclass, xml_parser_create(),
                  [1, 2], vec[3, 4], dict[1 => 2], keyset['abc']]);
  var_dump(varray[varray[], varray['a', 100, false]]);
  var_dump(varray[darray[], darray[100 => 'a', 'b' => 200]]);
}

function test_darray() {
  var_dump(darray[]);
  var_dump(darray[100 => 1, 200 => 2, 300 => 3, 400 => 4]);
  var_dump(darray['key1' => 'a', 'key2' => 'b', 'key3' => 'c']);
  var_dump(
    darray[
      10 => null,
      20 => true,
      30 => false,
      'key1' => 'abc',
      'key2' => 100,
      'key3' => 1.23,
      40 => new stdclass,
      50 => xml_parser_create(),
      60 => [1, 2],
      'key4' => vec[3, 4],
      'key5' => dict[1 => 2],
      70 => keyset['abc']
    ]
  );
  var_dump(darray['123' => 'abc', 123 => 'def']);
  var_dump(darray[123 => darray[], 456 => darray['a' => 100, 200 => 'b']]);
  var_dump(darray['abc' => varray[], 'def' => varray['a', 100, false]]);
}

test_varray();
test_darray();
