<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function convert_from($d) {
  echo "====================================================\n";
  var_dump($d);
  echo "----------------------------------------------------\n";
  var_dump((array)$d);
  var_dump(vec($d));

  try {
    var_dump(keyset($d));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  var_dump((bool)$d);
  var_dump((int)$d);
  var_dump((float)$d);
  var_dump((string)$d);
  var_dump((object)$d);
  var_dump(new Vector($d));
  var_dump(new Map($d));

  try {
    var_dump(new Set($d));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "====================================================\n";
}

function convert_to($from) {
  echo "====================================================\n";
  var_dump($from);
  echo "----------------------------------------------------\n";
  var_dump(dict($from));
  echo "====================================================\n";
}

function main() {
  convert_to([]);
  convert_to([100, 'val1', 'val2', 400, null, true, 1.234, new stdclass]);
  convert_to([1 => 10, 'key1' => 'val1', 5 => 'val2', 'key2' => 7,
              10 => null, 15 => true, 'key3' => 1.234,
              'key4' => new stdclass]);

  convert_to(vec[]);
  convert_to(vec[1, 2, 'a', 'b', 3, 4, false, null, 5.67, new stdclass]);

  convert_to(dict[]);
  convert_to(dict[1 => 10, 'key1' => 'val1', 5 => 'val2', 'key2' => 7,
                  10 => null, 15 => true, 'key3' => 1.234,
                  'key4' => new stdclass]);

  convert_to(keyset[]);
  convert_to(keyset[101, 202, 'val1', 'val2', 303]);
  convert_to(keyset[1, '1']);

  convert_to(false);
  convert_to(null);
  convert_to(123);
  convert_to(1.23);
  convert_to("abcd");
  convert_to(new stdclass);
  convert_to(Vector{1, 2, 3});
  convert_to(Map{'a' => 100, 200 => 'b'});
  convert_to(Pair{'a', 'b'});
  convert_to(Set{5, 4, 3, 2, 1});

  convert_from(dict[]);
  convert_from(dict[1 => 1, 2 => true, 3 => null, 4 => []]);
  convert_from(dict['key1' => 'val1', 'key2' => 'val2', 'key3' => 'val3']);
  convert_from(dict['key1' => 100, 'key2' => 'val1',
                    200 => 300, 400 => 'val2']);
  convert_from(dict['12345' => 100, 12345 => 200]);
  convert_from(dict[1 => dict[5 => 10], 'a' => dict['b' => 'c']]);
}

main();
