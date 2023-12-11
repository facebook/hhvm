<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_varray() :mixed{
  var_dump(vec[]);
  var_dump(vec[1, 2, 3, 4]);
  var_dump(vec['a', 'b', 'c']);
  var_dump(vec[null, true, false, 'abc', 100, 1.23,
                  new stdClass, xml_parser_create(),
                  vec[1, 2], vec[3, 4], dict[1 => 2], keyset['abc']]);
  var_dump(vec[vec[], vec['a', 100, false]]);
  var_dump(vec[dict[], dict[100 => 'a', 'b' => 200]]);
}

function test_darray() :mixed{
  var_dump(dict[]);
  var_dump(dict[100 => 1, 200 => 2, 300 => 3, 400 => 4]);
  var_dump(dict['key1' => 'a', 'key2' => 'b', 'key3' => 'c']);
  var_dump(
    dict[
      10 => null,
      20 => true,
      30 => false,
      'key1' => 'abc',
      'key2' => 100,
      'key3' => 1.23,
      40 => new stdClass,
      50 => xml_parser_create(),
      60 => vec[1, 2],
      'key4' => vec[3, 4],
      'key5' => dict[1 => 2],
      70 => keyset['abc']
    ]
  );
  var_dump(dict['123' => 'abc', 123 => 'def']);
  var_dump(dict[123 => dict[], 456 => dict['a' => 100, 200 => 'b']]);
  var_dump(dict['abc' => vec[], 'def' => vec['a', 100, false]]);
}


<<__EntryPoint>>
function main_literal() :mixed{
test_varray();
test_darray();
}
