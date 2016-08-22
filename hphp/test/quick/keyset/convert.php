<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function convert_from($ks) {
  echo "====================================================\n";
  var_dump($ks);
  echo "----------------------------------------------------\n";
  var_dump((array)$ks);
  var_dump(vec($ks));
  var_dump(dict($ks));
  var_dump((bool)$ks);
  var_dump((int)$ks);
  var_dump((float)$ks);
  var_dump((string)$ks);
  var_dump((object)$ks);
  var_dump(new Vector($ks));
  var_dump(new Map($ks));

  try {
    var_dump(new Set($ks));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "====================================================\n";
}

function convert_to($from) {
  echo "====================================================\n";
  var_dump($from);
  echo "----------------------------------------------------\n";
  try {
    var_dump(keyset($from));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  echo "====================================================\n";
}

function main() {
  convert_to([]);
  convert_to([100, 'val1', 'val2', 400, null, true, 1.234, new stdclass]);
  convert_to([100, 200, 'a', 'b']);
  convert_to(['1', 1]);
  convert_to(['abc', 123, 'abc', 123]);
  convert_to([1 => 10, 'key1' => 'val1', 5 => 'val2', 'key2' => 7,
              10 => null, 15 => true, 'key3' => 1.234,
              'key4' => new stdclass]);
  convert_to(['a' => 100, 'b' => 200, 100 => 'a', 200 => 'b']);
  convert_to(['a' => '1', 'b' => 1]);
  convert_to([100 => 'abc', 200 => 123, 300 => 'abc', 400 => 123]);

  convert_to(vec[]);
  convert_to(vec[1, 2, 'a', 'b', 3, 4, false, null, 5.67, new stdclass]);
  convert_to(vec[100, 'a', 200, 'b']);
  convert_to(vec['1', 1]);
  convert_to(vec[123, 'abc', 'abc', 123]);

  convert_to(dict[]);
  convert_to(dict[1 => 10, 'key1' => 'val1', 5 => 'val2', 'key2' => 7,
                  10 => null, 15 => true, 'key3' => 1.234, '1' => 20,
                  'key4' => new stdclass]);
  convert_to(dict['a' => 100, 'b' => 200, 100 => 'a', 200 => 'b']);
  convert_to(dict['a' => '1', 'b' => 1]);
  convert_to(dict[100 => 'abc', 200 => 123, 300 => 'abc', 400 => 123]);

  convert_to(keyset[]);
  convert_to(keyset[101, 202, 'val1', 'val2', 303]);
  convert_to(keyset['1', 1]);
  convert_to(keyset['abc', 123, 123, 'abc']);

  convert_to(false);
  convert_to(null);
  convert_to(123);
  convert_to(1.23);
  convert_to("abcd");
  convert_to(new stdclass);
  convert_to(Vector{1, 2, 3});
  convert_to(Vector{1, false, 3});
  convert_to(Map{'a' => 100, 200 => 'b'});
  convert_to(Map{'a' => 100, 200 => 1.2345});
  convert_to(Pair{'a', 'b'});
  convert_to(Pair{null, false});
  convert_to(Set{5, 4, 3, 2, 1});

  convert_from(keyset[]);
  convert_from(keyset[1, 2, 3, 4]);
  convert_from(keyset['z', 'y', 'x']);
  convert_from(keyset[1, '1', 2, '2']);
}

main();
