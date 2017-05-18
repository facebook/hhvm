<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a, $k) {
  echo "=============== get ================================\n";
  var_dump($a[$k]);
  echo "=============== isset===============================\n";
  var_dump(isset($a[$k]));
  echo "=============== akey-exists=========================\n";
  var_dump(array_key_exists($k, $a));
  echo "=============== idx ================================\n";
  var_dump(idx($a, $k));
  echo "=============== null-get ===========================\n";
  var_dump($a[$k] ?? 'def');
  echo "=============== set ================================\n";
  $a[$k] = 123;
  return $a;
}

function test_const_key($a) {
  echo "=============== get ================================\n";
  var_dump($a['2']);
  echo "=============== isset===============================\n";
  var_dump(isset($a['2']));
  echo "=============== akey-exists=========================\n";
  var_dump(array_key_exists('2', $a));
  echo "=============== idx ================================\n";
  var_dump(idx($a, '2'));
  echo "=============== null-get ===========================\n";
  var_dump($a['2'] ?? 'def');
  echo "=============== set ================================\n";
  $a['2'] = 123;
  return $a;
}

function run_tests() {
  test([], '10');
  test([1, 2, 3, 4], '2');
  test([10 => 'abc'], '10');

  test_const_key([]);
  test_const_key([1, 2, 3, 4]);
  test_const_key([2 => 'abc']);
}
run_tests();
