<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a, $k) :mixed{
  echo "=============== get ================================\n";
  try { var_dump($a[$k]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
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

function test_const_key($a) :mixed{
  echo "=============== get ================================\n";
  try { var_dump($a['2']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
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

function test_casting($a) :mixed{
  echo "======================= darray cast ================\n";
  var_dump(darray($a));
}

function run_tests() :mixed{
  test(dict[], '10');
  test(darray(vec[1, 2, 3, 4]), '2');
  test(darray(dict[10 => 'abc']), '10');

  test_const_key(dict[]);
  test_const_key(darray(vec[1, 2, 3, 4]));
  test_const_key(darray(dict[2 => 'abc']));

  test_casting(dict['1' => true, '2' => false]);
  test_casting(keyset['1', '2', '3']);

  echo "======================= literals ===================\n";
  var_dump(darray(dict['1' => true, '2' => false]));
  var_dump(darray(keyset['1', '2', '3']));
}

<<__EntryPoint>>
function main_intlike() :mixed{
run_tests();
}
